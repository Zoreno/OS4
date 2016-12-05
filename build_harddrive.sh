rm hard_drive.vdi

dd if=/dev/zero of=hard_drive.img bs=512 count=204800

mkdosfs -F 32 -I hard_drive.img

bash update_image.sh

bash create_harddrive.sh

rm hard_drive.img