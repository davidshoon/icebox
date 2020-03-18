#!/bin/sh

g++ -o recursive_sha256sum.out recursive_sha256sum.cpp -lcrypto

g++ -o delete_and_symlink_dup_shasum.out delete_and_symlink_dup_shasum.cpp

g++ -o list_dup_shasum.out list_dup_shasum.cpp

