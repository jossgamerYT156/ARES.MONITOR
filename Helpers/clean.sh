#	Helpers/clean.sh
#
#	Clean helper to remove files from bin/*
#	Makefile:@clean.target
#
#!/bin/env bash	
    for file in bin/*; do 
		if [ -f $file ]; then 
			rm $file; 
			echo "Removed $file"; 
		fi 
	done