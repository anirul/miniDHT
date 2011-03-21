/*
 * Copyright (c) 2009-2011, Frederic DUBOUCHET
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
 * THIS SOFTWARE IS PROVIDED BY Frederic DUBOUCHET ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Frederic DUBOUCHEDT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MINIDHT_DB_HEADER_DEFINED
#define MINIDHT_DB_HEADER_DEFINED

// STL
#include <fstream>
#include <map>
// BOOST
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
// BDB
#ifdef WITH_BDB
	#include "bdb_multibtree.h"
	#include "bdb_hash.h"
#endif // WITH_BDB

namespace miniDHT {
	
	template <
		class KEY_T,
		class VALUE_T,
		class COMPARE_T = std::less<KEY_T> >
	class db_wrapper : public
#ifdef WITH_BDB
		bdb::hash<KEY_T, VALUE_T, COMPARE_T>
#else
		std::map<KEY_T, VALUE_T, COMPARE_T>
#endif // WITH_BDB
	{
#ifndef WITH_BDB
		std::string file_name_;
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			std::map<KEY_T, VALUE_T, COMPARE_T>& map_data = (*this);
			ar & BOOST_SERIALIZATION_NVP(map_data);
		}
#endif // !WITH_BDB
	public :
		db_wrapper() :
#ifdef WITH_BDB
			bdb::hash<KEY_T, VALUE_T, COMPARE_T>()
#else
			std::map<KEY_T, VALUE_T, COMPARE_T>()
#endif // WITH_BDB
			{}
		virtual ~db_wrapper() {}
#ifndef WITH_BDB
		void open(const char* file_name) {
			file_name_ = std::string(file_name)
#ifdef SERIALIZE_XML
				+ std::string(".xml");
#endif // SERIALIZE_XML
#ifdef SERIALIZE_BINARY
				+ std::string(".bin");
#endif // SERIALIZE_BINARY
			std::ifstream ifs(file_name_.c_str());
			if (!ifs.is_open()) return;
#ifdef SERIALIZE_XML
			boost::archive::xml_iarchive arch(ifs);
			arch >> boost::serialization::make_nvp("db_wrapper", (*this));
#endif // SERIALIZE_XML
#ifdef SERIALIZE_BINARY
			boost::archive::binary_iarchive arch(ifs);
			arch >> (*this);
#endif // SERIALIZE_BINARY
			ifs.close();
		}
		void flush() {
			std::ofstream ofs(file_name_.c_str());
#ifdef SERIALIZE_XML
			boost::archive::xml_oarchive arch(ofs);
			arch << boost::serialization::make_nvp("db_wrapper", (*this));
#endif // SERIALIZE_XML
#ifdef SERIALIZE_BINARY
			boost::archive::binary_oarchive arch(ofs);
			arch << (*this);
#endif // SERIALIZE_BINARY
			ofs.close();
		}
		void close() {}
#endif // !WITH_BDB
	};
	
	template <
		class KEY_T,
		class VALUE_T,
		class COMPARE_T = std::less<KEY_T> >
	class db_multi_wrapper : public 
#ifdef WITH_BDB
		bdb::multibtree<KEY_T, VALUE_T, COMPARE_T>
#else
		std::multimap<KEY_T, VALUE_T, COMPARE_T>
#endif // WITH_BDB
	{
#ifndef WITH_BDB
		std::string file_name_;
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			std::multimap<KEY_T, VALUE_T, COMPARE_T>& multimap_data = (*this);
			ar & BOOST_SERIALIZATION_NVP(multimap_data);
		}
#endif // !WITH_BDB
	public :
		db_multi_wrapper() :
#ifdef WITH_BDB
			bdb::multibtree<KEY_T, VALUE_T, COMPARE_T>()
#else
			std::multimap<KEY_T, VALUE_T, COMPARE_T>()
#endif // WITH_BDB
			{}
		virtual ~db_multi_wrapper() {}
#ifndef WITH_BDB
		void open(const char* file_name) {
			file_name_ = std::string(file_name) 
#ifdef SERIALIZE_XML
				+ std::string(".xml");
#endif // SERIALIZE_XML
#ifdef SERIALIZE_BINARY
				+ std::string(".bin");
#endif // SERIALIZE_BINARY
			std::ifstream ifs(file_name_.c_str());
			if (!ifs.is_open()) return;
#ifdef SERIALIZE_XML
			boost::archive::xml_iarchive arch(ifs);
			arch >> boost::serialization::make_nvp("db_multi_wrapper", (*this));
#endif // SERIALIZE_XML
#ifdef SERIALIZE_BINARY
			boost::archive::binary_iarchive arch(ifs);
			arch >> (*this);
#endif // SERIALIZE_BINARY
			ifs.close();
		}
		void flush() {
			std::ofstream ofs(file_name_.c_str());
#ifdef SERIALIZE_XML
			boost::archive::xml_oarchive arch(ofs);
			arch << boost::serialization::make_nvp("db_multi_wrapper", (*this));
#endif // SERIALIZE_XML
#ifdef SERIALIZE_BINARY
			boost::archive::binary_oarchive arch(ofs);
			arch << (*this);
#endif // SERIALIZE_BINARY
			ofs.close();
		}
		void close() {}
#endif // !WITH_BDB
	};
}

#endif // MINIDHT_DB_HEADER_DEFINED