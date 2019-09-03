rm -f output
find "/bin" -iname "b*" -type f | xargs ls -l| awk '{print $9" "$3" "$1}' >> output
sed -i 's:/bin/::g' output
chmod 744 output