
#define msg(format,...) fprintf(stdout,"%s=(" format ")\n" ,#__VA_ARGS__,## __VA_ARGS__)
#define string_len(x)   (sizeof(x)-1)

int run_hash_fnv_test(int argc, ACE_TCHAR* argv[])
{
	FNV_1a<32> fnv_32;

    std::cout << std::hex;
    std::cout << "\n32bits test:\n";

    std::cout << "init:  'hello world' => fnv-32a :" << fnv_32("hello world",5) << std::endl;

	fnv_32.offset(FNV_1a<32>::INIT);

    std::cout << "reset: 'hello world' => fnv-32a :" << fnv_32("hello world",5) << std::endl;
    std::cout << "       'hello world' => fnv-32a :" << fnv_32( std::string("hello world")) << std::endl;
    std::cout << "       'hello world' => fnv-32a :" << fnv_32( std::string("hello world")) << std::endl;
    std::cout << "       'hello world' => fnv-32a :" << fnv_32( std::string("hello world")) << std::endl;
 
    std::cout << "\n64bits test:\n";

	FNV_1a<64> fnv_64;

    std::cout << "init:  'hello world' => fnv-64a :" << fnv_64("hello world",5) << std::endl;

	fnv_64.offset(FNV_1a<64>::INIT);

    std::cout << "reset: 'hello world' => fnv-64a :" << fnv_64("hello world",5) << std::endl;
    std::cout << "       'hello world' => fnv-64a :" << fnv_64( std::string("hello world")) << std::endl;
    std::cout << "       'hello world' => fnv-64a :" << fnv_64( std::string("hello world")) << std::endl;
    std::cout << "       'hello world' => fnv-64a :" << fnv_64( std::string("hello world")) << std::endl;

    std::cout << "\niovec test:\n";

    struct iovec io_test[2];

    char buff_a[] = "1234567890";
    char buff_b[] = "qwerty";
    char buff_c[] = "1234567890qwerty";


    msg("%s", buff_a);
    msg("%s", buff_b);
    msg("%s", buff_c);

    io_test[0].iov_base = buff_a;
    io_test[1].iov_base = buff_b;
    io_test[0].iov_len = string_len(buff_a);
    io_test[1].iov_len = string_len(buff_b);

    putchar('\n');

    msg("%s %d", (char *)io_test[0].iov_base, (int)io_test[0].iov_len);
    msg("%s %d", (char *)io_test[1].iov_base, (int)io_test[1].iov_len);

    putchar('\n');

	fnv_32.offset(FNV_1a<32>::INIT);
    msg("-> 0x%x <-", fnv_32(buff_a, string_len(buff_a)));
	fnv_32.offset(FNV_1a<32>::INIT);
    msg("-> 0x%x <-", fnv_32(buff_b, string_len(buff_b)));
	fnv_32.offset(FNV_1a<32>::INIT);
    msg("-> 0x%x <-", fnv_32(buff_c, string_len(buff_c)));

    std::cout << "\niovec:\n";
	fnv_32.offset(FNV_1a<32>::INIT);
    msg(">> 0x%x <<", fnv_32(io_test, 2));

    std::cout << "\n2-step test:\n";

	fnv_32.offset(FNV_1a<32>::INIT);
    msg("-> 0x%x <-", fnv_32(buff_a, string_len(buff_a)));
    msg(">> 0x%x <<", fnv_32(buff_b, string_len(buff_b)));

    putchar('\n');

	fnv_64.offset(FNV_1a<64>::INIT);
    msg("-> 0x%llx <-", static_cast<unsigned long long int>(fnv_64(buff_a, string_len(buff_a))));
    fnv_64.offset(FNV_1a<64>::INIT);
    msg("-> 0x%llx <-", static_cast<unsigned long long int>(fnv_64(buff_b, string_len(buff_b))));
    fnv_64.offset(FNV_1a<64>::INIT);
    msg("-> 0x%llx <-", static_cast<unsigned long long int>(fnv_64(buff_c, string_len(buff_c))));

    std::cout << "\niovec:\n";
    fnv_64.offset(FNV_1a<64>::INIT);
    msg(">> 0x%llx <<", static_cast<unsigned long long int>(fnv_64(io_test, 2)));

    std::cout << "\n2-step test:\n";
    fnv_64.offset(FNV_1a<64>::INIT);
    msg("-> 0x%llx <-", static_cast<unsigned long long int>(fnv_64(buff_a, string_len(buff_a))));
    msg(">> 0x%llx <<", static_cast<unsigned long long int>(fnv_64(buff_b, string_len(buff_b))));

	return 0;
}
