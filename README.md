# VirtualCC
Collective communication traffic simulator Based on The Network Simulator, Version 3

## Build
At top dir
```
./ns build
./ns run dgx
```

## Develop
1. cd mytopo
2. coding

## TODO
- rdma model
- collective communication apps like allreduce traffic
- nv dgx a100 2 nodes topo

- need to rebuild a bridge model to support p2p net devices
```
msg="Device does not support SendFrom: cannot be added to bridge.", +0.000000000s -1 file=/home/kozo/virtualCC/src/bridge/model/bridge-net-device.cc, line=258
```

## License

This software is licensed under the terms of the GNU General Public License v2.0 only (GPL-2.0-only). See the LICENSE file for more details.
