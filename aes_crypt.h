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
		memset(key, 0, 32);
		memset(iv, 0, 32);
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
		std::string out;
		int len = cleartext.size();
		int c_len = len + AES_BLOCK_SIZE;
		int f_len = 0;
		unsigned char *ciphertext = (unsigned char *)malloc(c_len);
		EVP_EncryptInit_ex(&e_ctx_, NULL, NULL, NULL, NULL);
		EVP_EncryptUpdate(
			&e_ctx_, 
			ciphertext, 
			&c_len, 
			(unsigned char const*)cleartext.c_str(), 
			len);
		EVP_EncryptFinal_ex(&e_ctx_, ciphertext + c_len, &f_len);
		len = c_len + f_len;
		out.resize(len);
		memcpy(&out[0], ciphertext, len);
		free(ciphertext);
		return out;
	}
	std::string decrypt(const std::string& ciphertext) {
		std::string out;
		int len = ciphertext.size();
		int p_len = len;
		int f_len = 0;
		unsigned char *plaintext = (unsigned char *)malloc(p_len);
		EVP_DecryptInit_ex(&d_ctx_, NULL, NULL, NULL, NULL);
		EVP_DecryptUpdate(
			&d_ctx_, 
			plaintext, 
			&p_len, 
			(unsigned char const*)ciphertext.c_str(), 
			len);
		EVP_DecryptFinal_ex(&d_ctx_, plaintext + p_len, &f_len);
		len = p_len + f_len;
		out.resize(len);
		memcpy(&out[0], plaintext, len);
		free(plaintext);
		return out;
	}
};

#endif // AES_CRYPT_HEADER_DEFINED

