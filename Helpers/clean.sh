#!/bin/env bash	
    for file in bin/*; do 
		if [ -f $file ]; then 
			rm $file; 
			echo "Removed $file"; 
		fi 
	done