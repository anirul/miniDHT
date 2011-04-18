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

#include <iostream>
#include <fstream>
#include <string>

#include <boost/program_options.hpp>

#include "aes_crypt.h"

int main(int ac, char** av) {
	std::string key;
	std::string input_file;
	std::string output_file = "out";
	bool encrypt = false;
	bool decrypt = false;
	try {
		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help message")
			("encode,e", "encode the text")
			("decode,d", "decode the text")
			("input-file,i", boost::program_options::value<std::string>(), 
				"file to encode or decode")
			("output-file,o", boost::program_options::value<std::string>(),
				"file output")
			("key,k", boost::program_options::value<std::string>(),
				"key to encode or decode")
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
			encrypt = true;
			decrypt = false;
		} else if (vm.count("decode")) {
			encrypt = false;
			decrypt = true;
		} else {
			std::cerr << "Should select either encrypt or decrypt" << std::endl;
			return -1;
		}
		if (vm.count("input-file")) {
			input_file = vm["input-file"].as<std::string>();
		} else {
			std::cerr << "Error you need an input file!" << std::endl;
			return -1;
		}
		if (vm.count("output-file")) {
			output_file = vm["output-file"].as<std::string>();			
		}
		if (vm.count("key")) {
			key = vm["key"].as<std::string>();
		} else {
			std::cerr << "Error you have to provide a key!" << std::endl;
			return -1;
		}
	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return -1;
	}
	FILE* ifd = fopen(input_file.c_str(), "rb");
	FILE* ofd = fopen(output_file.c_str(), "wb");
	if (!ifd) {
		std::cerr 
			<< "Error cannot open input file : " << input_file 
			<< std::endl;
		return -1;
	}
	if (!ofd) {
		std::cerr
			<< "Error cannot open output file : " << output_file
			<< std::endl;
		return -1;
	}
	try {
		aes_crypt<5, 42> ac(key);
		while (!feof(ifd)) {
			std::string buffer;
			buffer.resize(8 * 1024);
			int rd = fread(&buffer[0], 1, buffer.size(), ifd);
			buffer.resize(rd);
			if (encrypt) {
				buffer = ac.encrypt(buffer);
			} else if (decrypt) {
				buffer = ac.decrypt(buffer);
			}
			fwrite(&buffer[0], 1, buffer.size(), ofd);
		}
		std::string end_buffer = "";
		if (encrypt)
			end_buffer = ac.encrypt_end(end_buffer);
		else 
			end_buffer = ac.decrypt_end(end_buffer);
		fwrite(&end_buffer[0], 1, end_buffer.size(), ofd);
	} catch (const char* msg) {
		std::cerr << "Error : " << msg << std::endl;
		return -1;
	}
	return 0;
}
