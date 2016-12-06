#include <vfs/file_system.h>

#include <lib/string.h>

#define DEVICE_MAX 26

PFILESYSTEM _FileSystems[DEVICE_MAX];

char* fs_err_str(FS_ERROR error){

	switch(error){
		case FSE_GOOD:
			return "FSE_GOOD";
		case FSE_DEVICE_NOT_PRESENT:
			return "FSE_DEVICE_NOT_PRESENT";
		case FSE_BAD_FILE:
			return "FSE_BAD_FILE";
		case FSE_FILE_NOT_FOUND:
			return "FSE_FILE_NOT_FOUND";
		case FSE_DIR_NOT_FOUND:
			return "FSE_DIR_NOT_FOUND";
		case FSE_INVALID_FILE_NAME:
			return "FSE_INVALID_FILE_NAME";
		case FSE_FILE_CORRUPT:
			return "FSE_FILE_CORRUPT";
		case FSE_EOF:
			return "FSE_EOF";
		case FSE_BAD_FILESYSTEM:
			return "FSE_BAD_FILESYSTEM";
		default:
			return "Unexpected error";
	}

};

FS_ERROR fs_open_file(PFILE file, const char* fileName, uint32_t flags){
	if(!fileName){
		file->flags = FS_INVALID;
		return FSE_INVALID_FILE_NAME;
	}

	unsigned char device = 'a';

	char* fName = (char*) fileName;

	// a:/file.ext
	// b:/dir/file.ext
	// Default is a:

	// /file.ext <=> file.ext <=> a:/file.ext

	if(fileName[1] == ':'){ // If second char is ':', first is device.
		device = fileName[0];
		fName += 2; // Read past these two signs
	}

	// Read past eventual leading '/'
	if(fName[0] == '/'){
		fName += 1;
	}

	if(_FileSystems[device - 'a']){
		file->deviceID = device;

		return _FileSystems[device - 'a']->f_open(file, fName, flags);
	}

	file->flags = FS_INVALID;
	return FSE_DEVICE_NOT_PRESENT;
}

FS_ERROR fs_read_file(PFILE file, void* buffer, size_t length){
	if(file && (file->flags != FS_INVALID)){
		if(_FileSystems[file->deviceID - 'a']){
			return _FileSystems[file->deviceID - 'a']->f_read(file,buffer,length);
		}
		return FSE_DEVICE_NOT_PRESENT;
	}
	return FSE_BAD_FILE;
}

FS_ERROR fs_write_file(PFILE file, const void* buffer, size_t length){
	if(file && (file->flags != FS_INVALID)){
		if(_FileSystems[file->deviceID - 'a']){
			return _FileSystems[file->deviceID - 'a']->f_write(file,buffer,length);
		}
		return FSE_DEVICE_NOT_PRESENT;
	}
	return FSE_BAD_FILE;
}

FS_ERROR fs_close_file(PFILE file){
	if(file && (file->flags != FS_INVALID)){
		if(_FileSystems[file->deviceID - 'a']){
			return _FileSystems[file->deviceID - 'a']->f_close(file);
		}
		return FSE_DEVICE_NOT_PRESENT;
	}
	return FSE_BAD_FILE;
}

FS_ERROR fs_register(PFILESYSTEM fileSystem, unsigned int deviceID){
	if((deviceID < 'z') && (deviceID >= 'a')){
		if(!fileSystem){
			return FSE_BAD_FILESYSTEM;
		}

		_FileSystems[deviceID - 'a'] = fileSystem;

		return FSE_GOOD;
	}

	return FSE_DEVICE_NOT_PRESENT;
}

FS_ERROR fs_unregister(PFILESYSTEM fileSystem){
	for(int i = 0; i < DEVICE_MAX; ++i){
		if(_FileSystems[i] == fileSystem){
			_FileSystems[i] = 0;

			return FSE_GOOD;
		}
	}

	return FSE_DEVICE_NOT_PRESENT;
}

FS_ERROR fs_unregister_by_id(unsigned int deviceID){
	if((deviceID - 'a') < DEVICE_MAX){
		_FileSystems[deviceID-'a'] = 0;
		return FSE_GOOD;
	}

	return FSE_DEVICE_NOT_PRESENT;
}