# for each file in examples, builds and executes bin/main
for name in $(ls -1 examples); do 
cp examples/$name in.txt && make > /dev/null 2>&1 && echo -e "\n\n\n$name:" && cat examples/$name && echo && ./bin/main -a -i > /dev/stdout; done
