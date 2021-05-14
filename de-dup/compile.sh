#!/bin/sh

g++ -g -O3 -o recursive_sha256sum.out recursive_sha256sum.cpp -lcrypto

g++ -g -O3 -o recursive_suid.out recursive_suid.cpp -lcrypto

g++ -g -O3 -o recursive_filesize.out recursive_filesize.cpp

g++ -g -O3 -o delete_and_symlink_dup_shasum.out delete_and_symlink_dup_shasum.cpp

g++ -g -O3 -o list_dup_shasum.out list_dup_shasum.cpp

g++ -g -O3 -o copy_original_shasum.out copy_original_shasum.cpp

