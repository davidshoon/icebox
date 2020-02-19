/*
	Super-duper blockchain code.

	By David Shoon

	Copyright (c) 2020 - Licensed Under the MIT license.

	Base off the python code: https://medium.com/crypto-currently/lets-build-the-tiniest-blockchain-e70965a248b
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <list>

#include <openssl/sha.h>

struct Block
{
	Block(int index, time_t timestamp, const char *data, const char *prev_hash);

	int index_;
	time_t timestamp_;
	char data_[256];
	char prev_hash_[20];
	char hash_[20];
};

char *hash_block(char *block, size_t len)
{
	SHA_CTX shacontext;

	SHA1_Init(&shacontext);

	SHA1_Update(&shacontext, block, len);
	static unsigned char sha1digest[20] = {0};

	SHA1_Final(sha1digest, &shacontext);

/*	printf("SHA1 digest: ");

	for (int i = 0; i < 20; i++)
	{
		printf("%02x", sha1digest[i]);
	}

	printf("\n");
*/
	return (char *) sha1digest;
}

Block::Block(int index, time_t timestamp, const char *data, const char *prev_hash) : index_{index}, timestamp_{timestamp}
{
	for (int i = 0; i < sizeof(data_); i++) {
		data_[i] = data[i];
	}

	for (int i = 0; i < sizeof(prev_hash_); i++) {
		prev_hash_[i] = prev_hash[i];
	}

	char *hash = hash_block(reinterpret_cast <char *> (this), sizeof(*this));

	for (int i = 0; i < sizeof(hash_); i++) {
		hash_[i] = hash[i];
	}
}

Block next_block(Block &last_block)
{
	char str[256];

	snprintf(str, sizeof(str), "Hey I'm block %d", last_block.index_ + 1);

	return Block(last_block.index_ + 1, time(NULL), str, last_block.hash_);
}

int main()
{
	const char *genesis_block_data = "Genesis block";
	const char *genesis_block_hash = "01234567890123456789";
	Block genesis_block(0, time(NULL), genesis_block_data, genesis_block_hash);
	std::list <Block> blockchain;
	Block &previous_block = genesis_block;

	blockchain.push_back(genesis_block);

	for (int i = 0; i < 20; i++) {
		Block new_block = next_block(previous_block);
		blockchain.push_back(new_block);
		previous_block = new_block;

		printf("Block #%d: has been added to the blockchain!\n", new_block.index_);
		printf("Hash: ");
		for (int j = 0; j < 20; j++) {
			printf("%02x", (unsigned char) new_block.hash_[j]);
		}
		printf("\n");
	}

}
