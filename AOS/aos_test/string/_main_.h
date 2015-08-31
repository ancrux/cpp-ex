
#include "stx/btree_set" // namespace stx
#include "stx/btree_map" // namespace stx
#include <set> // namespace std
#include <map> // namespace std
#include <hash_map> // namespace stlport
#include <vector>

// custom comparator for ordering
struct less_cstr
{
	bool operator() (const char* cstr1, const char* cstr2)
	{
		return ::strcmp(cstr1, cstr2) < 0;
	}
};

// custom comparator for ordering
struct greater_cstr
{
	bool operator() (const char* cstr1, const char* cstr2)
	{
		return ::strcmp(cstr1, cstr2) >= 0;
	}
};


int run_sorter_test(int argc, ACE_TCHAR* argv[])
{
	aos::Multi_String mstr;
	mstr.push_back("orange");
	mstr.push_back("apple");
	mstr.push_back("banana");

	typedef std::set< const char* , less_cstr > SORTER;
	SORTER sorter;
	sorter.insert(mstr[0]);
	sorter.insert(mstr[1]);
	sorter.insert(mstr[2]);

	for(SORTER::iterator iter = sorter.begin(); iter != sorter.end(); ++iter)
	{
		ACE_OS::printf("%s\n", *iter);
	}

	// can also use std::sort() or std::stable_sort to sort
	// std::vector, std::dequeue, or even native C array like int A[]
	// because these containers have random access iterators
	// std::list has its own sort() function,
	// don't use std::sort with std::list or std::map families (slow)

	return 0;
}

int run_matcher_test(int argc, ACE_TCHAR* argv[])
{
	Char_Node node;
	::printf("sizeof_class:%d\r\n", node.sizeof_class());

	Char_Node_T< double > nodeT;
	::printf("sizeof_classT:%d\r\n", nodeT.sizeof_class());


	static const int N_ITEM = 20 * 1000 * 1000;
	static const int N_BUCKET = 256; //256 * 256;

	/*
	do
	{
		::printf("std::vector insert %d items:\n", N_ITEM * 10);
		std::vector< int > vec;
		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM * 10; ++n)
		{
			vec.push_back(n);
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		vec.clear();
	}
	while(0);
	//*/

	/*
	do
	{
		::printf("Char_Indexer insert %d items:\n", N_ITEM);
		Char_Indexer* cindexer = new Char_Indexer();
		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM; ++n)
		{
			//char buf[256];
			//_itoa(n, buf, 10);
			//cindexer.insert_key(buf);

			char* buf = (char*) &n;
			(*cindexer).insert_key(buf, sizeof(n));
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		// search
		::printf("Char_Indexer search %d items:\n", N_ITEM);
		ACE_Time_Value s1 = ACE_OS::gettimeofday();
		for(int n = 2; n >= 0; --n)
		{
			char* cstr = (char*) &n;
			(*cindexer).search(cstr, sizeof(n));
		}
		ACE_Time_Value s2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", s2.msec()-s1.msec());
		::printf("hit=%d\r\n", Char_Indexer::count);

		delete cindexer;
	}
	while(0);
	//*/

	/*
	do
	{
		::printf("std::map insert %d items:\n", N_ITEM);
		typedef std::map< int, int > ipv4;
		ipv4 map;
		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM; ++n)
		{
			map.insert(std::make_pair(n, 1));
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		// search
		::printf("search %d items:\n", N_ITEM);
		int n_hit = 0;
		ACE_Time_Value s1 = ACE_OS::gettimeofday();
		for(int n = N_ITEM; n > 0; --n)
		{
			ipv4::iterator iter = map.find(n);
			if ( iter != map.end() ) ++n_hit;
		}
		ACE_Time_Value s2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d, hit= %d\n", s2.msec()-s1.msec(), n_hit);

		// iteration
		::printf("iteration %d items:\n", N_ITEM);
		ACE_Time_Value i1 = ACE_OS::gettimeofday();
		ipv4::iterator iter = map.begin();
		while( iter != map.end() )
		{
			//ACE_OS::printf("[%u]=%u\n", iter->first, iter->second);
			++iter;
		}
		ACE_Time_Value i2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", i2.msec()-i1.msec());

		::printf("hit a key...\n"); getchar();

		map.clear();
	}
	while(0);
	//*/

	/*
	do
	{

		::printf("%d std::map insert %d items:\n", N_BUCKET, N_ITEM);
		typedef std::map< int, char > ipv4;
		std::vector< ipv4* > maps;
		for(int n = 0; n < N_BUCKET; ++n)
			maps.push_back(new ipv4);

		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM; ++n)
		{
			char* buf = (char*) &n;
			int i = ((unsigned char) *buf) * 255 + ((unsigned char) *(buf+1));
			(*maps[i]).insert(std::make_pair(n, 1));
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		for(int n = 0; n < N_BUCKET; ++n)
			delete maps[n];
		maps.clear();
	}
	while(0);
	//*/

	/*
	do
	{
		::printf("stlport::map insert %d items:\n", N_ITEM);
		typedef stlport::map< int, char > ipv4;
		ipv4 map;
		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM; ++n)
		{
			map.insert(stlport::make_pair(n, 1));
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		// search
		::printf("search %d items:\n", N_ITEM);
		int n_hit = 0;
		ACE_Time_Value s1 = ACE_OS::gettimeofday();
		for(int n = N_ITEM; n > 0; --n)
		{
			ipv4::iterator iter = map.find(n);
			if ( iter != map.end() ) ++n_hit;
		}
		ACE_Time_Value s2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d, hit= %d\n", s2.msec()-s1.msec(), n_hit);

		// iteration
		::printf("iteration %d items:\n", N_ITEM);
		ACE_Time_Value i1 = ACE_OS::gettimeofday();
		ipv4::iterator iter = map.begin();
		while( iter != map.end() )
		{
			//ACE_OS::printf("[%u]=%u\n", iter->first, iter->second);
			++iter;
		}
		ACE_Time_Value i2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", i2.msec()-i1.msec());

		::printf("hit a key...\n"); getchar();

		map.clear();
	}
	while(0);
	//*/


	/*
	do
	{
		::printf("std::map insert %d items:\n", N_ITEM);
		typedef std::map< int, char > ipv4;
		ipv4 map;
		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM; ++n)
		{
			map.insert(std::make_pair(n, 1));
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		// search
		::printf("search %d items:\n", N_ITEM);
		int n_hit = 0;
		ACE_Time_Value s1 = ACE_OS::gettimeofday();
		for(int n = N_ITEM; n > 0; --n)
		{
			ipv4::iterator iter = map.find(n);
			if ( iter != map.end() ) ++n_hit;
		}
		ACE_Time_Value s2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d, hit= %d\n", s2.msec()-s1.msec(), n_hit);

		//::printf("hit a key...\n"); getchar();

		map.clear();
	}
	while(0);
	//*/

	/*
	do
	{
		::printf("stlport::hash_map insert %d items:\n", N_ITEM);
		typedef stlport::hash_map< int, int > ipv4;
		ipv4 map;
		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM; ++n)
		{
			map.insert(stlport::make_pair(n, 1));
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		// search
		::printf("search %d items:\n", N_ITEM);
		int n_hit = 0;
		ACE_Time_Value s1 = ACE_OS::gettimeofday();
		for(int n = N_ITEM; n > 0; --n)
		{
			ipv4::iterator iter = map.find(n);
			if ( iter != map.end() ) ++n_hit;
		}
		ACE_Time_Value s2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d, hit= %d\n", s2.msec()-s1.msec(), n_hit);

		// iteration
		::printf("iteration %d items:\n", N_ITEM);
		ACE_Time_Value i1 = ACE_OS::gettimeofday();
		ipv4::iterator iter = map.begin();
		while( iter != map.end() )
		{
			//ACE_OS::printf("[%u]=%u\n", iter->first, iter->second);
			++iter;
		}
		ACE_Time_Value i2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", i2.msec()-i1.msec());

		::printf("hit a key...\n"); getchar();

		map.clear();
	}
	while(0);
	//*/

	///*
	// BEST performance in insertion
	do
	{
		::printf("stx::btree_map insert %d items:\n", N_ITEM);
		typedef stx::btree_map< int, char > ipv4;
		ipv4 map;
		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM; ++n)
		{
			map.insert(std::make_pair(n, 1));
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		// search
		::printf("search %d items:\n", N_ITEM);
		int n_hit = 0;
		ACE_Time_Value s1 = ACE_OS::gettimeofday();
		//ACE_Thread_Mutex lock_; //@
		for(int n = N_ITEM; n > 0; --n)
		{
			//ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, -1); //@

			ipv4::iterator iter = map.find(n);
			if ( iter != map.end() ) ++n_hit;
		}
		ACE_Time_Value s2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d, hit= %d\n", s2.msec()-s1.msec(), n_hit);

		// iteration
		::printf("iteration %d items:\n", N_ITEM);
		ACE_Time_Value i1 = ACE_OS::gettimeofday();
		ipv4::iterator iter = map.begin();
		while( iter != map.end() )
		{
			//ACE_OS::printf("[%u]=%u\n", iter->first, iter->second);
			++iter;
		}
		ACE_Time_Value i2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", i2.msec()-i1.msec());

		int key = -1;
		map.insert(std::make_pair(key, 2));
		map.insert(std::make_pair(key, 3));
		ipv4::iterator it = map.find(key);
		ACE_OS::printf("key[%u]=%u\n", it->first, it->second);

		::printf("hit a key...\n"); getchar();

		map.clear();
	}
	while(0);
	//*/

	/*
	do
	{
		::printf("%d stx::btree_map insert %d items:\n", N_BUCKET, N_ITEM);
		typedef stx::btree_map< ACE_UINT32, char > ipv4;
		std::vector< ipv4* > maps;
		for(int n = 0; n < N_BUCKET; ++n)
			maps.push_back(new ipv4);

		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM; ++n)
		{
			char* buf = (char*) &n;
			int i = ((unsigned char) *buf);
			//int i = ((unsigned char) *buf) * 256 + ((unsigned char) *(buf+1));
			//ACE_UINT16 v = ((unsigned char) *(buf+2)) * 256 + ((unsigned char) *(buf+3));
			(*maps[i]).insert(std::make_pair(n, 1));
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		::printf("hit a key...\n"); getchar();

		size_t n_delete = 0;
		for(int n = 0; n < N_BUCKET; ++n)
		{
			n_delete += maps[n]->size();
			delete maps[n];
		}
		::printf("%u deleted\n", n_delete);

		maps.clear();
	}
	while(0);
	//*/

	/*
	do
	{
		::printf("stx::btree_set insert %d items:\n", N_ITEM);
		typedef stx::btree_set< int > ipv4;
		ipv4 map;
		ACE_Time_Value t1 = ACE_OS::gettimeofday();
		for(int n = 0; n < N_ITEM; ++n)
		{
			map.insert(n);
		}
		ACE_Time_Value t2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", t2.msec()-t1.msec());

		// search
		::printf("search %d items:\n", N_ITEM);
		int n_hit = 0;
		ACE_Time_Value s1 = ACE_OS::gettimeofday();
		//ACE_Thread_Mutex lock_; //@
		for(int n = N_ITEM; n > 0; --n)
		{
			//ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, lock_, -1); //@

			ipv4::iterator iter = map.find(n);
			if ( iter != map.end() ) ++n_hit;
		}
		ACE_Time_Value s2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d, hit= %d\n", s2.msec()-s1.msec(), n_hit);

		// iteration
		::printf("iteration %d items:\n", N_ITEM);
		ACE_Time_Value i1 = ACE_OS::gettimeofday();
		ipv4::iterator iter = map.begin();
		while( iter != map.end() )
		{
			//ACE_OS::printf("[%u]=%u\n", iter->first, iter->second);
			++iter;
		}
		ACE_Time_Value i2 = ACE_OS::gettimeofday();
		ACE_OS::printf("elasped:%d\n", i2.msec()-i1.msec());

		int key = -1;
		map.insert(2);
		map.insert(3);
		ipv4::iterator it = map.find(key);
		ACE_OS::printf("key[%u]=%u\n", *it, *it);

		::printf("hit a key...\n"); getchar();

		map.clear();
	}
	while(0);
	//*/

	//::printf("hit a key...\n"); getchar();

	return 0;
}

/*
#include "aos/String_Matcher.h"

#include <time.h>
#include "ace/OS.h"

#include <iostream>
#include <list>
#include <set>
#include <map>
//#include <boost/regex.hpp>

using namespace aos;
#include "_main_.h"


//template <int Slots>
//struct btree_traits_nodebug
//{
//static const bool       selfverify = true;
//static const bool       debug = false;
//
//static const int        leafslots = Slots;
//static const int        innerslots = Slots;
//};
//
//typedef std::string item_t;
//typedef stx::btree_multiset< item_t, std::less< item_t >, btree_traits_nodebug< 64 > > bt_multiset; 

int main(int argc, char* argv[])
{
aos::Char_Node node;
::printf("sizeof_class:%d\r\n", node.sizeof_class());

aos::Char_Node_T< double > nodeT;
::printf("sizeof_classT:%d\r\n", nodeT.sizeof_class());

//os::Char_Node* node = new os::Char_Node();
//for(int n=255; n >= 0; --n)
//{
//	//node->set((char) n, 0);
//}
//Char_Node* nodeC = node->set('C');
//Char_Node* nodeB = node->set('B');
////nodeB->set('E');
//Char_Node* nodeA = node->set('A');
//nodeA->set('D');
//
//node->unset('B');
//os::Char_Node* p = node->find('B');
//::printf("%p\r\n", p);

////node->clear();
////node->dump();

//delete node;
//::printf("sizeof:%d\r\n", sizeof(os::Char_Node));


//::printf("---\r\n");
////search();
//os::Char_Indexer idx;
//idx.insert_key("ABCDE");

FILE* fp = 0;
char* book = 0;
fp = ::fopen("3200.txt", "r");
if ( fp )
{
::fseek(fp, 0, SEEK_END);
int fsize = ::ftell(fp);
::printf("filesize:%d\r\n", fsize);
::fseek(fp, 0, SEEK_SET);

book = new char[fsize+1];
book[fsize] = '\0';
size_t n_read = ::fread(book, sizeof(char), fsize, fp);

::fclose(fp);
}
//book[10000000] = '\0';

const char* text = "SXABCDE";
const char* key = "ABCD";
const char* key2 = "ABCE";

ACE_Time_Value ks = ACE_OS::gettimeofday();

Char_Indexer cindexer;

std::string keys;
typedef unsigned int item_t;
std::set< item_t > set;
std::map< item_t, item_t > map;

// for cindexer
for(int n = 0; n < 1000000; ++n)
{
char buf[256];
_itoa(n, buf, 10);
cindexer.insert_key(buf);
}

// for regex
//for(int n = 0; n < 10000; ++n)
//{
//	char buf[256];
//	_itoa(n, buf, 10);
//	if ( n == 0 )
//		keys += buf;
//	else 
//	{
//		keys += "|";
//		keys += buf;
//	}
//}

// for cindexer 2
//::printf("sizeof(int): %d\r\n", sizeof(int));
//for(int n = 0; n < 1000000; ++n)
//{
//	char buf[256];
//	::memcpy(buf, &n, sizeof(n));
//	//::printf("%X %X %X %X\r\n", buf[0], buf[1], buf[2], buf[3]); //@
//	cindexer.insert_key(buf, sizeof(n));
//}

// for std::set, map
//std::string strbuf(256, '\0');
//for(size_t n = 0; n < 10000000; ++n)
//{
//	//_itoa(n, &strbuf[0], 10);
//	set.insert(n);
//	//map.insert(std::make_pair<item_t, item_t>(n, n));
//}

::printf("sizeof(keys):%d\r\n",keys.size());

//::printf("keys:%s\r\n",keys.c_str());

//cindexer.insert_key("ABCDE");
//cindexer.insert_key("ABCD");
//cindexer.insert_key("BCD");
//cindexer.insert_key("BCE");
//cindexer.insert_key("SX");

for(int i = 0; i < 256; ++i)
{
char buf[2];
buf[1] = '\0';
buf[0] = (char) i;
//::printf("%X %X\r\n", buf[0], buf[1]); //@
cindexer.insert_key(buf);
}

cindexer.insert_key("Tom");
cindexer.insert_key("Sawyer");
cindexer.insert_key("Huckleberry");
cindexer.insert_key("Finn");

ACE_Time_Value ke = ACE_OS::gettimeofday();
printf("diff=%d\r\n", ke.msec()-ks.msec());

char* s = book;
std::string str(book);


//size_t tlen = 40960000; // 40mb
//char* s = (char*) ::malloc(tlen);
//
//memset(s, '1', tlen);
////strncpy(s, text, 7);
//strncpy(s+tlen-10, text, 7);
//
//s[tlen-1] = '\0';


//boost::regex regex(keys+"Tom|Sawyer|Huckleberry|Finn");
////boost::regex regex("Twain");
////boost::cmatch what;
//boost::match_results<std::string::const_iterator> what; 



printf("press key...\r\n");
int n = 0; // n of matches
getchar();
ACE_Time_Value ts = ACE_OS::gettimeofday();

//// boost:regex
//
//char m[256];
//std::string::const_iterator start, end; 
//   start = str.begin(); 
//   end = str.end(); 
//boost::match_flag_type flags = boost::match_default;
//while( boost::regex_search(start, end, what, regex, flags) )
//{
//	//int msize = what[0].second-what[0].first;
//	//strncpy(m, what[0].first, msize);
//	//m[msize] = '\0';
//	//::printf("match:%s\r\n", m);
//	//book = (char*) what[0].second;
//	//std::cout << what[0] << endl;
//	start = what[0].second;
//	++n;
//}
//

//
//for(int i=0; i < 1000; ++i)
//{
//	if ( ::strstr(s, "ABCDE") )
//		;//::printf("strstr found!\r\n");
//}
//


//int i = 0;
//while( *s )
//{
//	cindexer.move(s);
//	s++;
//	i++;
//}
//
////cindexer.search(s);
//::printf("Char_Indexer::count=%d\r\n", Char_Indexer::count);

//os::Char_Map cm;
//cm.set('C');
//
//int i = 0;
//while( *s )
//{
//	int res = cm.has(*s);
//	if ( res ) ::printf("C\r\n");
//	s++;
//	i++;
//}

ACE_Time_Value te = ACE_OS::gettimeofday();

//printf("i=%d\r\n", i);
printf("n=%d\r\n", n);
printf("diff=%d\r\n", te.msec()-ts.msec());

return 0;
}
//*/

int run_trim_test(int argc, ACE_TCHAR* argv[])
{
	std::string str("   sdfsxdd \v\xFF\v\0");
	ACE_OS::printf("before:[%s]\n", str.c_str());
	aos::trim(str);
	//aos::trim(str, &Char_Map("\0\xFF\v ", 4));
	//aos::trim(str, 'd');
	//aos::toupper(str);
	ACE_OS::printf("after:[%s]\n", str.c_str());

	return 0;
}

int run_char_map_test(int argc, ACE_TCHAR* argv[])
{
	// Char_Map Test
	Char_Map cm;
	cm.set('A');
	cm |= ' ';
	cm |= '\t';
	cm |= '\r';
	cm |= '\n';
	cm.set(" \t\r\n");
	Char_Map cm2 = cm;
	//cm2 = cm;
	cm.has('A');
	cm.dump();
	printf("\r\n");
	cm2.dump();
	cm.invert();
	printf("\r\n");
	cm.dump();
	printf("\r\n");
	cm.invert();
	cm.dump();

	::printf("\r\n");

	return 0;

	/* // Usage: to test a set of characters in Char_Map
	os::Char_Map cm;
	cm.set('A');
	cm |= ' ';
	cm |= '\t';
	cm |= '\r';
	cm |= '\n';
	cm.has('A');
	cm.dump();
	// */
};

int run_char_tokenizer_test(int argc, ACE_TCHAR* argv[])
{
	// Char_Tokenizer Test
	aos::Tokenizer tok("ABCD\r\nABC\r\nAB");
	tok.set_separator("\r\n");

	for(;;) // while( (int res = tok.next()) > Tokenizer::End )
	{
		int res = tok.next();
		// if ( res < 0 ) break; // only return token ended with separators
		if ( res < -1 ) break; // include last token that ends with str end

		::printf("<");
		std::string token(tok.token(), tok.size());
		::printf("%s", token.c_str());
		::printf(">(%d)\r\n", res);
	}

	return 0;

	/* // Usage:
	os::Tokenizer tok(mb.base());
	tok.set_separator("\r\n");

	for(;;) // while( (int res = tok.next()) > Tokenizer::End )
	{
	int res = tok.next();
	// if ( res < 0 ) break; // only return token ended with separators
	if ( res < -1 ) break; // include last token that ends with str end

	::printf("<");
	::write(1, tok.token(), (unsigned int) tok.size());
	::printf(">(%d)\r\n", res);
	}
	// */
};

int run_slist_test(int argc, ACE_TCHAR* argv[])
{
	std::list<int> dlist;
	for(int i=0; i < 3; ++i)
		dlist.push_front(i);

	std::list<int>::iterator it = dlist.begin();
	while( it != dlist.end() )
	{
		::printf("d=%d\r\n", *it);
		if ( *it % 2 == 0 && *it < 10 )
		{
			dlist.insert(it, (*it) * 100);
		}
		++it;
	}
	//::printf("\r\n");
	//it = dlist.begin();
	//while( it != dlist.end() )
	//{
	//	if ( *it > 10 )
	//	{
	//		dlist.erase(it++);
	//	}
	//	else
	//		++it;
	//}
	// delete all
	//it = dlist.begin();
	//while( it != dlist.end() )
	//{
	//	dlist.erase(it++);
	//	//++it;
	//}
	::printf("dlist items:\r\n");
	it = dlist.begin();
	while( it != dlist.end() )
	{
		::printf("d=%d\r\n", *it);
		++it;
	}
	::printf("\r\n");

	SList<int> slist;
	for(int i=0; i < 3; ++i)
		slist.push_front(i);

	SList<int>::iterator iter = slist.begin();
	while( iter != slist.end() )
	{
		::printf("s=%d\r\n", *iter);
		if ( *iter % 2 == 0 && *iter < 10 )
		{
			slist.insert(iter, (*iter) * 100);
		}
		++iter;
	}
	::printf("\r\n");
	iter = slist.begin();
	while( iter != slist.end() )
	{
		if ( *iter > 10 )
		{
			slist.erase(iter);
		}
		else
			++iter;
	}
	// delete all
	iter = slist.begin();
	while( iter != slist.end() )
	{
		slist.erase(iter);
		//++iter;
	}
	::printf("slist items:\r\n");
	iter = slist.begin();
	while( iter != slist.end() )
	{
		::printf("s=%d\r\n", *iter);
		++iter;
	}

	return 0;
}
