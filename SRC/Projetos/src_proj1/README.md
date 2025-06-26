compactar e dividir
```
tar -cf - src_proj1/ | split -b 100M - src_proj1.tar.part-
```
para juntar tudo
```
cat src_proj1.tar.part-* > src_proj1.tar && tar -xf src_proj1.tar
```

