/*
 * Copyright (c) 2011, anirul
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

#include "base64.h"
#ifdef BASE64_TEST_MAIN
#include <boost/program_options.hpp>
#include <stdio.h>
#include <string>
#endif // BASE64_TEST_MAIN

std::string encode_base64(const std::string& in) {
	std::string out = "";
	BIO *bmem, *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, &in[0], in.size());
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	out.resize(bptr->length);
	memcpy(&out[0], bptr->data, bptr->length-1);
	out[bptr->length-1] = 0;

	BIO_free_all(b64);

	return out;
}

std::string decode_base64(const std::string& in) {
	std::string out = "";
	BIO *b64, *bmem;

	out.resize(in.size());
	memset(&out[0], 0, out.size());

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new_mem_buf((void*)&in[0], (int)in.size());
	bmem = BIO_push(b64, bmem);
	BIO_flush(b64);

	BIO_read(bmem, &out[0], out.size());

	BIO_free_all(bmem);

	return out;
}

#ifdef BASE64_TEST_MAIN
int main(int ac, char** av) {
	bool encode = true;
	bool decode = false;
	std::string file_name = "";
	try {
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "produce help message")
        ("encode,e", "encode")
        ("decode,d", "decode")
        ("file,f", boost::program_options::value<std::string>(),
			"file to be encoded or decoded")
        ;
        boost::program_options::variables_map vm;
        boost::program_options::store(
			boost::program_options::command_line_parser(
				ac,
				av).options(desc).run(),
			vm);
        boost::program_options::notify(vm);
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 1;
        }
        if (vm.count("encode")) {
        	encode = true;
        	decode = false;
        }
        if (vm.count("decode")) {
        	encode = false;
        	decode = true;
        }
        if (vm.count("file")) {
        	file_name = vm["file"].as<std::string>();
        } else {
        	throw "No input file.";
        }
        {
        	std::string buffer = "";
        	FILE* file = NULL;
        	file = fopen(file_name.c_str(), "rb");
        	if (!file)
        		throw "Invalid file to be send.";
        	fseeko(file, 0, SEEK_END);
        	size_t total_size = ftello(file);
        	fseeko(file, 0, SEEK_SET);
        	buffer.resize(total_size);
        	int rest = fread(&buffer[0], 1, total_size, file);
        	if (rest != total_size)
        		throw "Could not read the full file.";
        	if (encode)
        		std::cout << encode_base64(buffer);
        	if (decode)
        		std::cout << decode_base64(buffer);
        }
	} catch(std::exception& ex) {
		std::cerr << "exception : " << ex.what() << std::endl;
	} catch(const char* msg) {
		std::cerr << "exception : " << msg << std::endl;
	}
	return 0;
}
#endif // BASE64_TEST_MAIN
