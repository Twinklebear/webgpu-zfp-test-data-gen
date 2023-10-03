# ZFP Fixed Rate Data Compressor App

This app can be used to compress raw volume files for rendering with
[webgpu-prog-iso](https://github.com/Twinklebear/webgpu-prog-iso)
and [webgpu-bcmc](https://github.com/Twinklebear/webgpu-bcmc).
It requires [ZFP](https://github.com/LLNL/zfp).

The app takes the volume file and the compression rate. The compression
rate is specified in the number of bits per-value to use in the compressed
representation. For example, 1 would mean represent each value with a single bit.

The volume files should follow the naming convention:

```
<name>_<x_size>x<y_size>x<z_size>_<voxel_type>.raw
```

The `x_size`, `y_size`, and `z_size` specify the grid dimensions of the volume,
`voxel_type` specifies the type of each cell value.
Supported `voxel_type`s are: `uint8`, `uint16`, `float32`, `float64`.

