"""
Point cloud generator: create random points inside 3 volumetric shapes (default 2,000,000 points).

Shapes supported:
  a) Ball (uniform in volume), default radius 10
  b) Cube (uniform in volume), default side length 10
  c) Cube with a through cylindrical hole (default hole radius 1, axis z), cube side 10

Output:
	- If Open3D is installed: save PLY (binary), and include normals if provided
	- Otherwise:
		- Without normals: save XYZ (text) and additionally NPY (binary)
		- With normals: save XYZN (text: x y z nx ny nz) and additionally NPZ (points, normals)

Usage examples:
	- Without normals:
			python generate_sphere.py --shape ball --N 2000000 --radius 10 --out ball_2M
			python generate_sphere.py --shape cube --N 2000000 --side 10 --out cube_2M
			python generate_sphere.py --shape cube_hole --N 2000000 --side 10 --hole-radius 1 --axis z --out cube_hole_2M
	- With normals:
			python generate_sphere.py --shape ball --N 2000000 --radius 10 --normals --out ball_2M_n
			python generate_sphere.py --shape cube --N 2000000 --side 10 --normals --out cube_2M_n
			python generate_sphere.py --shape cube_hole --N 2000000 --side 10 --hole-radius 1 --axis z --normals --out cube_hole_2M_n

Note: 2,000,000 x 3 float32 ≈ 24 MB. If memory is tight, consider --dtype float16 or chunked saving.
"""

from __future__ import annotations

import argparse
import time
import math
import os
import sys
from typing import Literal, Tuple

import numpy as np

try:
	import open3d as o3d  # type: ignore
	_HAS_O3D = True
except Exception:
	o3d = None  # type: ignore
	_HAS_O3D = False


# --------------------------- Generation functions ---------------------------
def generate_ball_points(N: int, radius: float, rng: np.random.Generator) -> np.ndarray:
	"""Uniformly sample N points inside a ball of given radius.

	Method: sample random directions by normalizing Gaussian vectors;
	radius r = R * U^(1/3) to ensure uniformity in volume.
	Returns: (N, 3) float32 array.
	"""
	# Random directions
	dirs = rng.normal(size=(N, 3)).astype(np.float64)
	norms = np.linalg.norm(dirs, axis=1, keepdims=True)
	dirs /= np.clip(norms, 1e-12, None)

	# Radius distribution: U^(1/3)
	r = radius * np.cbrt(rng.random(N, dtype=np.float64))
	pts = dirs * r[:, None]
	return pts.astype(np.float32)


def generate_cube_points(N: int, side: float, rng: np.random.Generator) -> np.ndarray:
	"""Uniformly sample N points inside cube [-s/2, s/2]^3."""
	half = side / 2.0
	pts = rng.uniform(low=-half, high=half, size=(N, 3)).astype(np.float32)
	return pts


# --------------------------- Normals computation (optional) ---------------------------
def _safe_normalize(v: np.ndarray, eps: float = 1e-12) -> np.ndarray:
	"""Row-wise normalize (N, 3) vectors safely, avoiding zero-length.
	Returns unit vectors with the same shape; tiny-norm rows are set to 0.
	"""
	n = np.linalg.norm(v, axis=1, keepdims=True)
	# 先做安全除法（用 eps 防止除零），随后将长度过小的行置零
	# Safe division (clip to avoid division by zero), then zero-out tiny-norm rows
	out = v / np.clip(n, eps, None)
	mask = (n > eps)[:, 0]
	out[~mask] = 0.0
	return out


def compute_normals_ball(points: np.ndarray) -> np.ndarray:
	"""SDF gradient direction inside the ball (outward): n = normalize(p)."""
	return _safe_normalize(points.astype(np.float64)).astype(np.float32)



def compute_normals_cube(points: np.ndarray, side: float) -> np.ndarray:
	"""Normals inside the cube volume (using the box SDF gradient approximation).

	For interior points, SDF = max(qx, qy, qz) where q = |p| - b and b = side/2.
	The gradient points to the nearest face: along argmax(q) with sign sign(p[i]).
	"""
	half = side / 2.0
	p = points.astype(np.float64)
	q = np.abs(p) - half
	# argmax over axis=1 gives the axis index of the nearest face
	idx = np.argmax(q, axis=1)
	signs = np.sign(p[np.arange(p.shape[0]), idx])
	normals = np.zeros_like(p)
	normals[np.arange(p.shape[0]), idx] = signs
	return normals.astype(np.float32)


def compute_normals_cube_with_cyl_hole(points: np.ndarray, side: float, hole_radius: float, axis: Literal["x", "y", "z"]) -> np.ndarray:
	"""Normals inside a "cube minus cylinder" volume using SDF composition: max(sdf_box, -sdf_cyl).

	- If sdf_box > -sdf_cyl, the nearest boundary is the outer cube surface: use box normal.
	- Otherwise, the nearest boundary is the inner cylindrical wall: use -grad(sdf_cyl) (pointing into the hole, consistent with outward normal).
	"""
	half = side / 2.0
	p = points.astype(np.float64)
	# Box SDF (negative inside): sdf_box = max(qx, qy, qz)
	q = np.abs(p) - half
	sdf_box = np.max(q, axis=1)
	# Cylinder SDF (positive outside, negative inside): rho - R
	if axis == "z":
		rho = np.sqrt(p[:, 0] ** 2 + p[:, 1] ** 2)
		grad_cyl = np.stack([p[:, 0], p[:, 1], np.zeros_like(p[:, 2])], axis=1)
	elif axis == "y":
		rho = np.sqrt(p[:, 0] ** 2 + p[:, 2] ** 2)
		grad_cyl = np.stack([p[:, 0], np.zeros_like(p[:, 1]), p[:, 2]], axis=1)
	else:  # axis == 'x'
		rho = np.sqrt(p[:, 1] ** 2 + p[:, 2] ** 2)
		grad_cyl = np.stack([np.zeros_like(p[:, 0]), p[:, 1], p[:, 2]], axis=1)
	sdf_cyl = rho - hole_radius

	# Choose which SDF dominates
	use_box = sdf_box >= -sdf_cyl

	# Box normals
	normals = np.zeros_like(p)
	if np.any(use_box):
		idx = np.argmax(q[use_box], axis=1)
		rows = np.where(use_box)[0]
		signs = np.sign(p[rows, idx])
		normals[rows, idx] = signs

	# Cylindrical normals: -grad(sdf_cyl) direction = -(grad rho)
	if np.any(~use_box):
		grad = _safe_normalize(grad_cyl[~use_box])
		normals[~use_box] = -grad

	return normals.astype(np.float32)


def _print_progress(prefix: str, current: int, total: int, width: int = 30) -> None:
	"""Lightweight progress bar printed on a single line."""
	current = min(current, total)
	ratio = current / total if total > 0 else 1.0
	pct = int(ratio * 100)
	filled = int(ratio * width)
	bar = "#" * filled + "-" * (width - filled)
	print(f"\r{prefix} [{bar}] {pct:3d}% ({current:,}/{total:,})", end="", flush=True)


def generate_cube_with_cyl_hole(
	N: int,
	side: float,
	hole_radius: float,
	axis: Literal["x", "y", "z"],
	rng: np.random.Generator,
	show_progress: bool = True,
) -> np.ndarray:
	"""Uniformly sample points inside a cube while removing a centered through cylinder.

	Cube bounds: [-side/2, side/2]^3.
	Cylinder axis: axis (x/y/z), radius hole_radius, spanning the cube.
	Uses rejection sampling; kept volume fraction is high (~97%), so it's efficient.
	Returns: (N, 3) float32.
	"""
	half = side / 2.0

	# Estimate kept fraction to size the candidate batch
	cyl_volume = math.pi * (hole_radius ** 2) * side
	cube_volume = side ** 3
	keep_ratio = max(1e-6, 1.0 - cyl_volume / cube_volume)

	pts_list = []
	got = 0
	# Candidate batch size (slightly inflated to reduce loop count)
	batch = max(10_000, int(math.ceil((N - got) / keep_ratio)))

	if show_progress:
		_print_progress("采样立方体(含孔)", 0, N)

	while got < N:
		cand = rng.uniform(low=-half, high=half, size=(batch, 3))
		x, y, z = cand[:, 0], cand[:, 1], cand[:, 2]
		if axis == "z":
			mask = (x * x + y * y) >= (hole_radius * hole_radius)
		elif axis == "y":
			mask = (x * x + z * z) >= (hole_radius * hole_radius)
		else:  # axis == "x"
			mask = (y * y + z * z) >= (hole_radius * hole_radius)

		kept = cand[mask]
		pts_list.append(kept)
		got += kept.shape[0]

		if show_progress:
			_print_progress("采样立方体(含孔)", got, N)

		# Adjust batch dynamically to avoid extremes
		remaining = N - got
		if remaining > 0:
			batch = max(10_000, int(math.ceil(remaining / keep_ratio)))

	pts = np.concatenate(pts_list, axis=0)[:N]
	if show_progress:
		_print_progress("采样立方体(含孔)", N, N)
		print()  # newline
	return pts.astype(np.float32)


# --------------------------- Saving functions ---------------------------
def save_points(points: np.ndarray, base_path: str, normals: np.ndarray | None = None) -> Tuple[str, str | None]:
	"""Save point clouds.
	Prefer PLY (binary, requires Open3D, include normals if provided).
	Fallbacks:
	- Without normals: XYZ (text) and NPY (binary)
	- With normals: XYZN (text) and NPZ (points, normals)

	Returns: (main_path, aux_path)
	main_path: primary output (PLY or XYZ/XYZN)
	aux_path: path to NPY/NPZ if written, else None
	"""
	main_path = os.path.abspath(base_path)
	aux_path: str | None = None

	if _HAS_O3D:
		# Save as PLY (binary)
		ply_path = main_path if main_path.lower().endswith(".ply") else f"{main_path}.ply"
		pcd = o3d.geometry.PointCloud()
		pcd.points = o3d.utility.Vector3dVector(points.astype(np.float64))
		if normals is not None:
			pcd.normals = o3d.utility.Vector3dVector(normals.astype(np.float64))
		o3d.io.write_point_cloud(ply_path, pcd, write_ascii=False)
		return ply_path, None

	# Without Open3D: choose format based on whether normals exist
	if normals is None:
		xyz_path = main_path if main_path.lower().endswith(".xyz") else f"{main_path}.xyz"
		npy_path = f"{main_path}.npy"
		# XYZ text (large, but widely supported)
		np.savetxt(xyz_path, points, fmt="%.6f")
		# NPY binary (compact)
		np.save(npy_path, points)
		aux_path = npy_path
		return xyz_path, aux_path
	else:
		xyzn_path = main_path if main_path.lower().endswith(".xyzn") else f"{main_path}.xyzn"
		npz_path = f"{main_path}.npz"
		# XYZN text: x y z nx ny nz
		arr = np.hstack([points.astype(np.float64), normals.astype(np.float64)])
		np.savetxt(xyzn_path, arr, fmt="%.6f")
		# NPZ binary
		np.savez_compressed(npz_path, points=points, normals=normals)
		aux_path = npz_path
		return xyzn_path, aux_path


# --------------------------- CLI ---------------------------
def parse_args(argv: list[str]) -> argparse.Namespace:
	parser = argparse.ArgumentParser(description="生成三类体内随机点云")
	parser.add_argument("--shape", choices=["ball", "cube", "cube_hole"], required=True,
						help="选择形状：ball/cube/cube_hole")
	parser.add_argument("--N", type=int, default=2_000_000, help="点的数量，默认 2,000,000")
	parser.add_argument("--radius", type=float, default=10.0, help="球半径 (shape=ball)")
	parser.add_argument("--side", type=float, default=10.0, help="立方体边长 (shape=cube/cube_hole)")
	parser.add_argument("--hole-radius", dest="hole_radius", type=float, default=1.0,
						help="圆柱孔半径 (shape=cube_hole)")
	parser.add_argument("--axis", choices=["x", "y", "z"], default="z",
						help="圆柱孔轴向 (shape=cube_hole)，默认 z")
	parser.add_argument("--seed", type=int, default=42, help="随机种子，默认 42")
	parser.add_argument("--dtype", choices=["float32", "float16"], default="float32",
						help="输出精度，默认 float32")
	parser.add_argument("--normals", action="store_true",
						help="为每个点计算并保存法线（基于形状 SDF 梯度）")
	parser.add_argument("--out", type=str, default=None,
						help="输出文件基名（不带扩展名）。省略则自动生成")
	parser.add_argument("--no-progress", dest="no_progress", action="store_true",
						help="禁用进度显示")
	return parser.parse_args(argv)


def main(argv: list[str]) -> int:
	args = parse_args(argv)
	rng = np.random.default_rng(args.seed)
	show_progress = not getattr(args, "no_progress", False)

	# 生成
	if args.shape == "ball":
		start_t = time.time()
		print("生成球体点云…", end="", flush=True)
		pts = generate_ball_points(args.N, args.radius, rng)
		print(f" 完成，用时 {time.time()-start_t:.2f}s")
		default_name = f"ball_r{args.radius:g}_{args.N//1_000_000}M"
	elif args.shape == "cube":
		start_t = time.time()
		print("生成立方体点云…", end="", flush=True)
		pts = generate_cube_points(args.N, args.side, rng)
		print(f" 完成，用时 {time.time()-start_t:.2f}s")
		default_name = f"cube_s{args.side:g}_{args.N//1_000_000}M"
	else:  # cube_hole
		start_t = time.time()
		pts = generate_cube_with_cyl_hole(args.N, args.side, args.hole_radius, args.axis, rng, show_progress=show_progress)
		print(f"采样完成，用时 {time.time()-start_t:.2f}s")
		default_name = f"cube_hole_s{args.side:g}_r{args.hole_radius:g}_{args.axis}_{args.N//1_000_000}M"

	# 计算法线（可选）
	normals: np.ndarray | None = None
	if getattr(args, "normals", False):
		calc_t = time.time()
		print("计算法线…", end="", flush=True)
		if args.shape == "ball":
			normals = compute_normals_ball(pts)
		elif args.shape == "cube":
			normals = compute_normals_cube(pts, args.side)
		else:  # cube_hole
			normals = compute_normals_cube_with_cyl_hole(pts, args.side, args.hole_radius, args.axis)
		print(f" 完成，用时 {time.time()-calc_t:.2f}s")

	# 精度
	if args.dtype == "float16":
		pts = pts.astype(np.float16)
		if normals is not None:
			normals = normals.astype(np.float16)
	else:
		pts = pts.astype(np.float32)
		if normals is not None:
			normals = normals.astype(np.float32)

	# 输出
	base = args.out or default_name
	print("保存中…", end="", flush=True)
	path, aux = save_points(pts, base, normals=normals)
	print(" 已保存")

	print(f"已生成点云：shape={args.shape}, N={args.N:,}")
	print(f"主文件：{path}")
	if aux:
		print(f"辅助文件：{aux}")
	print("坐标范围（min->max）：")
	mins = pts.min(axis=0)
	maxs = pts.max(axis=0)
	print(f"  x: {mins[0]:.3f} ~ {maxs[0]:.3f}")
	print(f"  y: {mins[1]:.3f} ~ {maxs[1]:.3f}")
	print(f"  z: {mins[2]:.3f} ~ {maxs[2]:.3f}")
	if normals is not None:
		# 简要检查法线长度
		lens = np.linalg.norm(normals.astype(np.float64), axis=1)
		print(f"法线长度：min={lens.min():.3f}, max={lens.max():.3f}, mean={lens.mean():.3f}")
	return 0


if __name__ == "__main__":
	raise SystemExit(main(sys.argv[1:]))