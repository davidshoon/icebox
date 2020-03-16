#!/bin/sh

g++ -o blockchain.out blockchain.cpp -lcrypto

g++ -o delete_and_symlink_dup_shasum.out delete_and_symlink_dup_shasum.cpp

g++ -o recursive_sha256sum.out recursive_sha256sum.cpp -lcrypto
