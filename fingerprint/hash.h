/*
 * Copyright (C) 2016 The Android Open Source Project
 * Copyright (C) 2016 The CyanogenMod Project
 * Copyright (C) 2016 The Mokee Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __HASH_H_
#define __HASH_H_


#define SHA1CircularShift(bits,word) ((((word) << (bits)) & 0xFFFFFFFF) | ((word) >> (32-(bits))))
#define MAX_FILE_LENGTH (300000)

typedef struct SHA1Context{
	unsigned Message_Digest[5];
	unsigned Length_Low;
	unsigned Length_High;
	unsigned char Message_Block[64];
	int Message_Block_Index;
	int Computed;
	int Corrupted;
} SHA1Context;

void SHA1Reset(SHA1Context *);
int SHA1Result(SHA1Context *);
void SHA1Input( SHA1Context *,const char *,unsigned);

void SHA1ProcessMessageBlock(SHA1Context *);
void SHA1PadMessage(SHA1Context *);
int sha1_hash(const char *source, int len);

uint64_t hash_file(const char *file_path);
uint64_t hash_string(const char *str);

#endif //__HASH_H_
