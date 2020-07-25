# MCUTL unit-testing
MCUTL allows you to unit-test your project (and also tests itself) on a host PC. You don't need any physical hardware to test your firmware code. **GTest** and **GMock** are used for testing. These libraries are bundled with MCUTL, but they are not required when compiling for MCU.

To enable testing on a host, define the `MCUTL_TEST` macro. When this macro is defined, all [memory access functions](memory.md) and all MCU-specific instruction calls are routed via a mock interface layer using GMock. Then build the code using a host compiler (g++ or clang).

The interface and the mock for memory accesses are located in the **mcutl/tests/volatile_memory.h** file. This file is automatically included and used when the `MCUTL_TEST` macro is defined. The interface used to mock the memory operations is extremely simple:
```cpp
class memory_interface
{
public:
	virtual ~memory_interface() {}

public:
	virtual void write(memory_address_t address, uint64_t value) = 0;
	virtual uint64_t read(memory_address_t address) = 0;
};
```

The mock for this interface is called `memory_interface_mock`. There are also two basic test fixtures `test_fixture_base` and `strict_test_fixture_base` which both utilize this mock.

#### memory_interface_mock
```cpp
class memory_interface_mock : public memory_interface
{
public:
	//Initializes mock with default behavior. Hashmap is used as a fake memory container,
	//and by default read and write operations interact with this hashmap.
	//allow_all_reads parameter specifies if all reads should be allowed by default.
	//When this is true, GMock will allow any reads from any memory addresses, otherwise
	//only reads registered via EXPECT_CALL(read) or allow_reads() will be allowed.
	//Write operations are allowed only when expectations via EXPECT_CALL(write) are set.
	void initialize_default_behavior(bool allow_all_reads = true);
	
	//Returns a guard object. Until this object is deleted, all read and write calls will be
	//routed directly to the memory hashmap bypassing the GMock layer.
	memory_interface_not_mocked with_unmocked_memory() noexcept;
	
	//Allows any reads to the specific memory address.
	void allow_reads(memory_address_t address);
	
	//Returns value of the memory address.
	uint64_t get(memory_address_t address) const;
	
	//Sets value of the memory address.
	void set(memory_address_t address, uint64_t value);
	
	//Returns reference to value of the memory address.
	uint64_t& get(memory_address_t address);
	
	//Sets same address access limit. By default this limit is set to 50.
	//When code you are testing reads and writes the same memory address
	//more than 50 times in a row, an exception is thrown and the test fails.
	//This is required to deal with possible infinite loops, as firmware code
	//often waits for some values to show up in registers in infinite loops.
	void set_same_address_access_limit(uint32_t max_same_address_accesses);
};
```

#### test_fixture_base
```cpp
//This is the default test fixture which sets up the
//memory_interface_mock mock and can be used in tests.
//This test fixture allows all read operations.
class test_fixture_base : public ::testing::Test
{
public:
	//Returns memory_interface_mock instance.
	memory_interface_mock& memory() noexcept;

	//Converts any pointer to its address value.
	template<typename Pointer>
	static memory_address_t addr(const Pointer* pointer) noexcept;
};
```

#### strict_test_fixture_base
```cpp
//This is the default test fixture which sets up the
//memory_interface_mock mock and can be used in tests.
//This test fixture denies all read operations until
//expectations are set using EXPECT_CALL(read) or
//allow_reads() is called.
class strict_test_fixture_base : public test_fixture_base
{
};
```

There are already tests which test the library itself. They are located in the `tests` directory. You can use them as a reference to write your own firmware tests. The tests are built using `CMakeLists.txt` which is supplied with the library.