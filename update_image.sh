#!/bin/bash

sudo losetup /dev/loop0 hard_drive.img
sudo mount /dev/loop0 /mnt
sudo cp test.txt /mnt
sudo umount /dev/loop0
sudo losetup -d /dev/loop0 
