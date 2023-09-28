#!/bin/sh

qemu-system-x86_64 \
	-machine accel=kvm,type=q35 \
	-cpu host \
	-m 2G \
	-nographic \
	-device virtio-net-pci,netdev=net0 \
	-netdev user,id=net0,hostfwd=tcp::2222-:22 \
	-drive if=virtio,format=qcow2,file=ubuntu-20.04-server-cloudimg-amd64-disk-kvm.img \
	-drive if=virtio,format=raw,file=seed.img \
	# -kernel "build_folder/arch/x86/boot/bzImage" \
	# -append "root=/dev/root console=ttyS0,115200n8 nokasrl"
