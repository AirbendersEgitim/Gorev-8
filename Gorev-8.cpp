#include <atomic>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

class TestThreads {
public:
	/// Creates thread function which increases #a
	/// @see #thread_func_increase_a
	TestThreads();
	/// Joins background thread
	~TestThreads();

	// non-copyable
	TestThreads(const TestThreads&) = delete;
	TestThreads& operator=(const TestThreads&) = delete;
	// non-movable
	TestThreads(TestThreads&&) = delete;
	TestThreads& operator=(TestThreads&&) = delete;

	static constexpr std::chrono::milliseconds wait_time{ 700 };
	/// Unique ID of the class instance
	const int this_id;

private:
	/// @see #this_id
	static inline int last_id = 0;

	/**
	Increased value by all instances of this class in background.

	To avoid race conditions, the counter is atomic.
	If two threads try to increment the current value at the same time,	say they both get the value 5,
	they will both write 6 where the second one should have written 7.
	Atomic prevents such situations and acts like a mutex.

	@see #thread_func_increase_a
	*/
	static inline std::atomic<int> a{ 0 };

	/// Background thread which runs #thread_func_increase_a
	std::thread thr;

	/**
	Called by the thread of each class instance at construction

	Waits for #wait_time, then prints, and then waits for #wait_time to increment #a.
	*/
	static void thread_func_increase_a(const TestThreads* self, int increase_amount);
};

////////////////////////////////////////////////////////////////////////////////

int main()
{
	if (true) {
		std::vector<TestThreads> thrVec(4);
		std::cout << "Waiting for the parallel jobs to be finished" << std::endl;
	}
	std::cout << "All jobs finished! Terminating program...\n";
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

/// Avoid race conditions when writing to console (std::cout)
class PrintAtOnce {
public:
	PrintAtOnce() = default;
	/// Calls #flush
	~PrintAtOnce();

	// non-copyable
	PrintAtOnce(const PrintAtOnce&) = delete;
	PrintAtOnce& operator=(const PrintAtOnce&) = delete;
	// non-movable
	PrintAtOnce(PrintAtOnce&&) = delete;
	PrintAtOnce& operator=(PrintAtOnce&&) = delete;

	/// Prints the buffer to the console then resets the buffer
	void flush();

	/// Print to buffer
	/// @see #flush
	template <typename T>
	PrintAtOnce& operator<<(const T& x);

private:
	/// Buffer to be printed to console
	std::stringstream ss;
};

////////////////////////////////////////////////////////////////////////////////

PrintAtOnce::~PrintAtOnce()
{
	flush();
}

void PrintAtOnce::flush()
{
	std::cout << ss.str();
	ss = std::stringstream();
}

template <typename T>
PrintAtOnce& PrintAtOnce::operator<<(const T& x)
{
	ss << x;
	return *this;
}

////////////////////////////////////////////////////////////////////////////////

TestThreads::TestThreads()
	: this_id(++last_id)
	, thr(thread_func_increase_a, this, 5)
{
}

TestThreads::~TestThreads()
{
	thr.join();
	PrintAtOnce() << this_id << ") is terminated\n";
}

void TestThreads::thread_func_increase_a(const TestThreads* self, int increase_amount)
{
	for (int i = 0; i < increase_amount; i++) {
		PrintAtOnce msg;
		std::this_thread::sleep_for(wait_time);
		msg << self->this_id << ") increasing a\n";
		msg.flush();
		std::this_thread::sleep_for(wait_time);
		msg << self->this_id << ") a is increased to " << ++a << "\n";
	}
}
