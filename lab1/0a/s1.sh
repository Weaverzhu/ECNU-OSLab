mkdir -p foo
touch name.txt
touch stno.txt
echo "朱桐" > "./name.txt"
echo "10175102111" > "stno.txt"
cp -f "./name.txt" "./foo/name.txt"
cp -f "./stno.txt" "./foo/stno.txt"
