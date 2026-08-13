# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2025 Amlogic, Inc. All rights reserved.
#

#/bin/bash

declare BOARD_LIST_a9="by401:by408:by409"
declare BOARD_LIST_c4="bz400:bz401:bz404"
declare BOARD_LIST_t6w="bs301:bs309:bs311:bs319:bs331"
declare BOARD_LIST_t6x="bu301:bu309"

declare BL33_SOC_2025=("a9:c4")
declare BL33_SOC_2023=("t6w:t6x")

declare -a SUPPORT_BL33_VERSION=("2023" "2025")

declare  UBOOT_CHECK_PATH_2023="arch/arm/dts/amlogic:board/amlogic/configs:board/amlogic:configs/amlogic"
declare  UBOOT_CHECK_PATH_2025="arch/arm/dts/amlogic:include/configs/amlogic:board/amlogic:configs/amlogic"

declare -a UBOOT_CHECK_TYPE=("dts" "board_h" "board_c" "defconfig")

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

declare UBOOT_VERSION
declare UBOOT_PATH
declare OUT_PUT_FILES="$SCRIPT_DIR/check_result"



function usage() {
	cat << EOF
	Usage:
	$(basename $0) --help

	./$(basename $0) -u/--uboot 2023/2025

	out_put ./check_result/result.json
EOF
exit 1
}

function creat_jeson(){
	#$1: soc $2: board $3: check_type $4: git_info
	local soc="$1"
	local path="$3"
	local board="$2"
	local git_info="$4"

	jq	--arg s "$soc"	\
		--arg p "$path"	\
		--arg b "$board"	\
		--arg g "$git_info"	\
	'
		.[$s][$p][$b] += ($g | split("\n") | map(select(length > 0)))
	' $OUT_PUT_FILES/result.json > $OUT_PUT_FILES/tmp.json && mv $OUT_PUT_FILES/tmp.json $OUT_PUT_FILES/result.json
}

function get_file_info(){
	#$1: file_name $2: soc $3: board $4: check_type
	local git_info

	git_info=$(git log --oneline  --follow -n -20 -- "$1")
#| while read commit_hash rst  build_message
#	do
#		echo "$commit_hash"
#echo "===$commit_hash  $build_message==="
#		git diff-tree --no-commit-id --name-only -r $commit_hash
#		echo ""
#	done)
	creat_jeson $2 $3 $4 "$git_info"
}

function get_dir_info(){
	#$1: file_name $2: soc $3: board $4: check_type

	local git_info
	git_info=$(git log --oneline -100 $1)
#| while read commit_hash rst  build_message
#	do
#		echo "$commit_hash"
#	done)

	creat_jeson $2 $3 $4 "$git_info"
}

function get_commit_info () {
	# $1: soc $2: board $3: check_path $4: check_type
	cd $3
	if [ "dts" == "$4" ]; then
		file_name1="meson-$1-$2.dts"
		if [ -f "$file_name1" ]; then
			get_file_info $file_name1 $1 $2 $4
		fi

		file_name2="meson-$1-$2.dtsi"
		if [ -f "$file_name2" ]; then
			get_file_info $file_name2 $1 $2 $4
		fi
	elif [ "board_h" == "$4" ]; then
		file_name="$1_$2.h"

		if [ -f "$file_name" ]; then
			get_file_info $file_name $1 $2 $4
		fi
	elif [ "board_c" == "$4" ]; then
		dir_name="$1_$2"
		if [ -d "$dir_name" ]; then
			get_dir_info $dir_name $1 $2 $4
		fi
	elif [ "defconfig" == "$4" ]; then
		file_name="$1_$2_defconfig"
		if [ -f "$file_name" ]; then
			get_file_info $file_name $1 $2 $4
		fi

	fi

#	git log ./ --oneline -100 | while read commit_hash
#	do
#		local diff_info=`git diff-tree --no-commit-id --name-only -r HEAD`

#	done
}

function  process_redundant_info(){
	#$1: soc  $2: check_type $3 boards
	echo "$@"
	jq 	--arg soc "$1" \
		--arg path "$2" \
		--arg board "$3"	\
	'
	.[$soc][$path] as $all_boards |

	($all_boards | keys) as $boards |

	($boards | length) as $total |

	(reduce $boards[] as $b ({};
		reduce $all_boards[$b][] as $c (.;
			.[$c] += 1
		)
	)) as $counts |

	.[$soc][$path] = (
		reduce $boards[] as $b ({};
			.[$b] = [
				$all_boards[$b][] |
				select($counts[.] < $total)
			]
		)
	)
	' "$OUT_PUT_FILES/result.json" > "$OUT_PUT_FILES/tmp.json" && mv "$OUT_PUT_FILES/tmp.json" "$OUT_PUT_FILES/result.json"

}


function check() {
	local loop_soc
	local index
	local loop_board

	local soc_list1="BL33_SOC_$UBOOT_VERSION"
	eval soc_list2='$'$soc_list1
	local soc_list=(${soc_list2//:/ })

	local chec_path_list1="UBOOT_CHECK_PATH_$UBOOT_VERSION"
	eval check_path_list2='$'$chec_path_list1
	local check_path_list=(${check_path_list2//:/ })

	for loop_soc in ${soc_list[@]}; do #soc
		for index in ${!check_path_list[@]}; do #path
			local board_list1="BOARD_LIST_$loop_soc"
			eval board_list2='$'$board_list1
			local board_list=(${board_list2//:/ })

			for loop_board in ${board_list[@]}; do #board
				cd $UBOOT_PATH
				get_commit_info $loop_soc $loop_board ${check_path_list[$index]} ${UBOOT_CHECK_TYPE[$index]}
			done
			process_redundant_info $loop_soc ${UBOOT_CHECK_TYPE[$index]}
		done
	done
}

function get_path() {
	if [ "$UBOOT_VERSION" == "2023" ]; then
		UBOOT_PATH=$SCRIPT_DIR/../bl33/v2023/
	elif [ "$UBOOT_VERSION" == "2025" ]; then
		UBOOT_PATH=$SCRIPT_DIR/../bl33/v2025/
	fi

	cd $UBOOT_PATH
	UBOOT_PATH=`pwd`
	echo "uboot path: $UBOOT_PATH"
}

function Parse_para() {
	local i=0
	local argv=()

	for arg in "$@" ; do
		argv[$i]="$arg"
		i=$((i + 1))
	done

	i=0
	while [ $i -lt $# ]; do
		arg="${argv[$i]}"
		i=$((i + 1)) # must place here
		case "$arg" in
			-h|--h|--help)
				usage
				exit ;;
			-u|--u|--uboot)
				UBOOT_VERSION="${argv[$i]}"
				local tar=0
				for loop in ${SUPPORT_BL33_VERSION[@]}; do
					if [ "$loop" == "$UBOOT_VERSION" ]; then
						echo "### CHECK UBOOT VERSION is $UBOOT_VERSION"
						tar=1
						break;
					fi
				done
				if [ "$tar" == "0" ]; then
					echo "### $UBOOT_VERSION is not support bl33 ###"
					usage
					exit 1
				fi
				continue ;;
			*)
		esac
	done


}

function main(){
	if [ -z $1 ]
	then
		usage
		return
	fi

	Parse_para $@
	get_path
	if [ -e "$OUT_PUT_FILES" ]; then
		rm -rf $OUT_PUT_FILES
	fi
	mkdir -p $OUT_PUT_FILES
	echo "{}" > $OUT_PUT_FILES/result.json
	check
	jq . $OUT_PUT_FILES/result.json
}

main $@
