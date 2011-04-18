/*
 * Copyright (c) 2010, anirul
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the CERN nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY anirul ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL anirul BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AES_CRYPT_HEADER_DEFINED
#define AES_CRYPT_HEADER_DEFINED

#include <iostream>
#include <string>
#include <openssl/evp.h>

const int AES_BLOCK_SIZE = 256;

template <int T_ROUND = 5, unsigned char T_SALT = 42>
class aes_crypt { 
protected :
	EVP_CIPHER_CTX e_ctx_;
	EVP_CIPHER_CTX d_ctx_;
public :
	aes_crypt(const std::string& digest) {
		int i;
		unsigned char key[32];
		unsigned char iv[32];
		unsigned char salt = T_SALT;
		i = EVP_BytesToKey(
			EVP_aes_256_cbc(), 
			EVP_sha1(), 
			&salt,
			(unsigned char*)&digest[0],
			digest.size(),
			T_ROUND,
			key, iv);
		EVP_CIPHER_CTX_init(&e_ctx_);
		EVP_EncryptInit_ex(&e_ctx_, EVP_aes_256_cbc(), NULL, key, iv);
		EVP_CIPHER_CTX_init(&d_ctx_);
		EVP_DecryptInit_ex(&d_ctx_, EVP_aes_256_cbc(), NULL, key, iv);
	}
	std::string encrypt(const std::string& cleartext) {
//		EVP_EncryptInit_ex(&e_ctx_, NULL, NULL, NULL, NULL);
		int outl = cleartext.size() + AES_BLOCK_SIZE - 1;
		std::string ciphertext;
		ciphertext.resize(outl);
		EVP_EncryptUpdate(
			&e_ctx_,
			(unsigned char*)&ciphertext[0], 
			&outl,
			(unsigned char*)&cleartext[0],
			cleartext.size());
		ciphertext.resize(outl);
		return ciphertext;
	}
	std::string encrypt_end(const std::string& cleartext) {
		std::string ciphertext;
		ciphertext.resize(cleartext.size());
		int f_len = cleartext.size();
		EVP_EncryptFinal_ex(&e_ctx_, (unsigned char*)&ciphertext[0], &f_len);
		ciphertext.resize(f_len);
		return ciphertext;
	}
	std::string decrypt(const std::string& ciphertext) {
//		EVP_DecryptInit_ex(&d_ctx_, NULL, NULL, NULL, NULL);
		std::string cleartext;
		cleartext.resize(ciphertext.size());
		int p_len = ciphertext.size();
		EVP_DecryptUpdate(
			&d_ctx_,
			(unsigned char*)&cleartext[0],
			&p_len,
			(unsigned char*)&ciphertext[0],
			ciphertext.size());
		cleartext.resize(p_len);
		return cleartext;
	}
	std::string decrypt_end(const std::string& ciphertext) {
		std::string cleartext;
		cleartext.resize(ciphertext.size());
		int f_len = ciphertext.size();
		EVP_DecryptFinal(&d_ctx_, (unsigned char*)&cleartext[0], &f_len);
		cleartext.resize(f_len);
		return cleartext;
	}
};

#endif // AES_CRYPT_HEADER_DEFINED

