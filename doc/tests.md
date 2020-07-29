# MCUTL unit-testing
MCUTL allows you to unit-test your project (and also tests itself) on a host PC. You don't need any physical hardware to test your firmware code. **GTest** and **GMock** are used for testing. These libraries are bundled with MCUTL, but they are not required when compiling for MCU.

To enable testing on a host, define the `MCUTL_TEST` macro. When this macro is defined, all [memory access functions](memory.md) and all [MCU-specific instruction](instruction.md) calls are routed via a mock interface layer using GMock. Then build the code using a host compiler (g++ or clang).

## Memory mock and fixtures
The interface and the mock for memory accesses are located in the **mcutl/tests/volatile_memory.h** file. This file is automatically included and used when the `MCUTL_TEST` macro is defined. The interface used to mock the memory operations is extremely simple:
```cpp
//mcutl::tests::memory namespace
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
//mcutl::tests::memory namespace
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
//mcutl::tests::memory namespace
//This is the default test fixture which sets up the
//memory_interface_mock mock and can be used in tests.
//This test fixture allows all read operations.
class test_fixture_base : virtual public ::testing::Test
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
//mcutl::tests::memory namespace
//This is the default test fixture which sets up the
//memory_interface_mock mock and can be used in tests.
//This test fixture denies all read operations until
//expectations are set using EXPECT_CALL(read) or
//allow_reads() is called.
class strict_test_fixture_base : public test_fixture_base
{
};
```

## MCU-specific instructions mock and fixture
The interface and the mock for MCU-specific instructions execution are located in the **mcutl/tests/instruction.h** file. This file is automatically included and used when the `MCUTL_TEST` macro is defined. This is the interface used to mock MCU-specific instructions:
```cpp
//mcutl::tests::instruction namespace
class instruction_interface
{
public:
	virtual ~instruction_interface() {}

public:
	//Called when the instruction with the 'instruction_type' type is executed
	//with arguments 'args'
	virtual instruction_return_type run(
		std::type_index instruction_type,
		const instruction_args_type& args) = 0;
};
```

#### test_fixture_base
The library-provided mock of this interface is called `mcutl::tests::instruction::instruction_interface_mock`. There is also a library-provided test fixture for this mock:
```cpp
//mcutl::tests::instruction namespace
//This is the default test fixture which sets up the
//instruction_interface_mock mock and can be used in tests.
class test_fixture_base : virtual public ::testing::Test
{
public:
	//Returns instruction_interface_mock instance.
	[[nodiscard]] instruction_interface_mock& instruction() noexcept;
	
	//Converts Instruction to its std::type_index.
	template<typename Instruction>
	[[nodiscard]] static auto instr();
};
```

#### InstructionArgsEqual
As instruction arguments are represented as a `vector` of `any` objects, there is a helper GMock matcher to compare these arguments with the expected ones. This is an example of how to use it:
```cpp
TEST_F(instruction_test_fixture, InstructionArgsTest)
{
	constexpr const char* str = "test";
	
	//Checks if the 'wfe' MCU-specific instruction was called with arguments
	//123 (integer) and "test" (const char*).
	EXPECT_CALL(instruction(), run(instr<mcutl::device::instruction::type::wfe>(),
		mcutl::tests::instruction::InstructionArgsEqual(123, str)));
	mcutl::instruction::execute<mcutl::device::instruction::type::wfe>(123, str);
}
```

## MCU fixtures
There are library-provided MCU test fixtures, which include the above mentioned fixtures. You can access this mock by including the file `mcutl/tests/mcu.h`. There are two fixtures available: `mcutl::tests::mcu::test_fixture_base` and `mcutl::tests::mcu::strict_test_fixture_base`. The first one inherits `mcutl::tests::instruction::test_fixture_base` and `mcutl::tests::memory::test_fixture_base`, and the second one inherits `mcutl::tests::instruction::test_fixture_base` and `mcutl::tests::memory::strict_test_fixture_base`.

## Tests
There are already tests which test the library itself. They are located in the `tests` directory. You can use them as a reference to write your own firmware tests. The tests are built using `CMakeLists.txt` which is supplied with the library.