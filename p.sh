source=$1
dest=$2
addtext=$3
comment=$4

#echo "$source $dest"

echo "$(cp $source $dest)"
echo "obj += $addtext" >> $dest/Makefile

dir=$(pwd)
cd ~/gitrepo/imx6ull.code
git add .
git commit -a -m "$comment"
git push origin master
cd $dir
