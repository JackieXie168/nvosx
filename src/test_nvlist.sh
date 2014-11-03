#!/bin/sh
host_os=`uname -s`
if [ "$host_os" = "Linux" ]; then
	cfg=nvram
elif [ "$host_os" = "Darwin" -o  "$host_os" = "FreeBSD" ]; then
	cfg=xnvram
fi

if [ ! -e nvlist ]; then
	./autogen.sh && make
fi

$cfg unset vendor_confile_qid
for((i=1;i<=66;i++)); do ./nvlist -s vendor_confile_qid -a $i -p $i -d "___"; done
for((i=1;i<=66;i++)); do ./nvlist -s vendor_confile_qid -v $i -d "___"; done
echo "" && echo "Adding tokens to a empty list ..."
for((i=1;i<=65;i++)); do ./nvlist -s vendor_confile_qid -a $i -p $i -d ":::::"; done
echo "" && echo "Change delimiter(s) ..."
./nvlist -s vendor_confile_qid -o "---" -d ":::::"
echo "" && echo "To get the tokens from list ..."
for((i=1;i<=65;i++)); do ./nvlist -s vendor_confile_qid -i $i -d "---"; done
echo "" && echo "Change delimiter(s) ..."
./nvlist -s vendor_confile_qid -o "^_^" -d "---"
echo "" && echo "Reverse order the list for one by one from the ending	 ..."
for((i=1;i<=65;i++)); do ./nvlist -s vendor_confile_qid -m $i -p $(echo `expr 66 - $i`) -d "^_^"; done
echo "" && echo "Change delimiter(s) ..."
./nvlist -s vendor_confile_qid -o "-_-||" -d "^_^"
echo "" && echo "Reverse order the list for one by one again from the beginning ..."
for((i=1;i<=65;i++)); do ./nvlist -s vendor_confile_qid -m $i -p $i -d "-_-||"; done
echo "" && echo "Change delimiter(s) ..."
./nvlist -s vendor_confile_qid -o "@_@" -d "-_-||"
echo "" && echo "Remove tokens from the ending of list for one by one ..."
for((i=1;i<=65;i++)); do ./nvlist -s vendor_confile_qid -r $(echo `expr 66 - $i`) -d "@_@"; done
$cfg unset vendor_confile_qid
echo "" && echo "Insert tokens from the beginning of the empty list with bigger number then change by the smallest number for one by one ..."
for((i=1;i<=65;i++)); do ./nvlist -s vendor_confile_qid -a $(echo `expr 66 - $i`) -p $i -d ":::::"; ./nvlist -s vendor_confile_qid -m $i -p $i -d ":::::"; done
$cfg unset vendor_confile_qid
echo "" && echo "Test for changing delimiter(s) ..."
$cfg set tmp_nvram="0 11 222 3333 44444 555555 6666666 77777777 888888888 9999999999 10101010101010101010"
$cfg get tmp_nvram
./nvlist -s tmp_nvram -o "*" -d " "
./nvlist -s tmp_nvram -o "&&" -d "*"
./nvlist -s tmp_nvram -o "---" -d "&&"
./nvlist -s tmp_nvram -o "^^^" -d "---"
./nvlist -s tmp_nvram -o "~v~" -d "^^^"
./nvlist -s tmp_nvram -o " " -d "~v~"
./nvlist -s tmp_nvram -o "----" -d " "
./nvlist -s tmp_nvram -o "{[]}" -d "----"
./nvlist -s tmp_nvram -o "~*~" -d "{[]}"
./nvlist -s tmp_nvram -o "~*~" -d "~*~"
./nvlist -s tmp_nvram -o ":::" -d "~*~"
./nvlist -s tmp_nvram -o "**&&" -d ":::"
./nvlist -s tmp_nvram -o "###" -d "**&&"
./nvlist -s tmp_nvram -o "@@@" -d "###"
./nvlist -s tmp_nvram -o "|||" -d "@@@"
./nvlist -s tmp_nvram -o ";;;" -d "|||"
./nvlist -s tmp_nvram -o "~@~" -d ";;;"
./nvlist -s tmp_nvram -o "~v~^~^~" -d"~@~"
./nvlist -s tmp_nvram -o " " -d"~v~^~^~"
./nvlist -s tmp_nvram -o "--" -d" "
./nvlist -s tmp_nvram -o "---" -d"--"
./nvlist -s tmp_nvram -o "----" -d"---"
./nvlist -s tmp_nvram -o "--" -d"----"
./nvlist -s tmp_nvram -o " " -d"--"
./nvlist -s tmp_nvram -o "  " -d" "
./nvlist -s tmp_nvram -o "   " -d"  "
./nvlist -s tmp_nvram -o "    " -d"   "
./nvlist -s tmp_nvram -o "   " -d"    "
./nvlist -s tmp_nvram -o "  " -d"   "
./nvlist -s tmp_nvram -o " " -d"  "
./nvlist -s tmp_nvram -o "~" -d" "
./nvlist -s tmp_nvram -o " " -d"~"
./remove_shm.sh
