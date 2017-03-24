vboxmanage storageattach OS4 --storagectl IDE --port 0 --device 0 --medium none
vboxmanage closemedium hard_drive.vdi

rm hard_drive.vdi

dd if=/dev/zero of=hard_drive.img bs=512 count=204800

mkdosfs -F 32 -I hard_drive.img

bash update_image.sh

bash create_harddrive.sh

vboxmanage storageattach OS4 --storagectl IDE --port 0 --device 0 --type hdd --medium hard_drive.vdi
