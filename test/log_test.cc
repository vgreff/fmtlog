#include <chrono>
#include <iostream>
#include <sstream>
#include <array>

#include "../fmtlog.h"

struct Foo { int a; float b; };

template <>
struct fmt::formatter<Foo> : formatter<std::string>
{
  template <typename Context>
  auto format(const Foo &foo, Context &ctx) const
  {
    std::stringstream my_ss;
    my_ss << "a=" << foo.a << " b=" <<  foo.b << std::ends;
    
    return formatter<std::string>::format(my_ss.str(), ctx);
  }
};

//-----------------------------------------------------------------------------------

template <int Size>
class VgFixedBuffer
{
public:
  static constexpr const int MaxSize = Size;
  static constexpr const size_t SzSize = sizeof(size_t);
public:
  VgFixedBuffer();

  const char* data() const { return (char*)this; }
  size_t size() const { return size_ + SzSize;}

  template <typename T1>
  const T1* payloadDataAs() const { return (const T1*)(((char*)this) + SzSize); }
  
  const char* payloadData() const { return (((char*)this) + SzSize); }


  size_t payloadSize() const { return (size_ > SzSize) ?  size_ - SzSize : 0;}

  void load(char* data, size_t sz, size_t offset=0)
  {
    if (offset == 0) size_=0;
    memcpy(buffer_.data() + offset, data, sz);
    size_+=sz;
  }

  template <typename T1>
  void load(const T1& in, size_t offset=0)
  {
    load((char*)(&in), sizeof(T1), offset);
  }

  // bool useDataSize() const { return true;}
  bool useDataSize() const { return useDataSize_;}

  friend std::ostream& operator<< <>(std::ostream& out, const VgFixedBuffer<Size>& obj);
private:
  size_t size_{0};
  std::array<char, Size> buffer_;
public:
  bool useDataSize_{true};
};


template <int Size>
VgFixedBuffer<Size>::VgFixedBuffer()
{
}

template <int Size>
std::ostream& operator<<(std::ostream& out, const VgFixedBuffer<Size>& obj)
{
  out << "VgFixedBuffer<" << Size << ">:{ "
      << "size=" << obj.size_ << " "
      << "data.size=" << (uint64_t)obj.size() << " "
      << "data=" << (uint64_t)obj.data() << " ";

  out << "}\n";
  return out;
}
//-----------------------------------------------------------------------------------

using VgBuffer1024 = VgFixedBuffer<1024>;

template <>
struct fmt::formatter<VgBuffer1024> : formatter<std::string>
{
  template <typename Context>
  auto format(const VgBuffer1024& buf, Context &ctx) const
  {
    std::stringstream my_ss;
    // Foo* ptrf = (Foo*) buf.payloadData();
    auto foo = buf.payloadDataAs<Foo>();
    my_ss << "a=" << foo->a << " b=" <<  foo->b << std::ends;
    
    return formatter<std::string>::format(my_ss.str(), ctx);
  }
};

//-----------------------------------------------------------------------------------

struct MyType
{
  MyType(int val)
      : v(val) {}
  ~MyType()
  {
    dtor_cnt++;
    // fmt::print("dtor_cnt: {}\n", dtor_cnt);
  }
  int v{0};
  static int dtor_cnt;
};

int MyType::dtor_cnt = 0;

template <>
struct fmt::formatter<MyType> : formatter<int>
{
  // parse is inherited from formatter<string_view>.
  template <typename FormatContext>
  auto format(const MyType &val, FormatContext &ctx) const
  {
    return formatter<int>::format(val.v, ctx);
  }
};

struct MovableType
{
public:
  MovableType(int v = 0)
      : val{MyType(v)} {}
      // : val{MyType(v),MyType(v*2),MyType(v*4)} {}

  std::vector<MyType> val;
};

template <>
struct fmt::formatter<MovableType> : formatter<int>
{
  // parse is inherited from formatter<string_view>.
  template <typename FormatContext>
  auto format(const MovableType &val, FormatContext &ctx)
  {
    return formatter<int>::format(val.val[0].v, ctx);
  }
};

void runBenchmark();
void runBenchmark1();

void logcb(int64_t ns, fmtlog::LogLevel level, fmt::string_view location, size_t basePos, fmt::string_view threadName,
           fmt::string_view msg, size_t bodyPos, size_t logFilePos)
{
  fmt::print("callback full msg: {}, logFilePos: {}\n", msg, logFilePos);
  msg.remove_prefix(bodyPos);
  fmt::print("callback msg body: {}\n", msg);
}

void logQFullCB(void *userData)
{
  fmt::print("log q full\n");
}

int main()
{
  char randomString[] = "Hello World";
  logi("A string, pointer, number, and float: '{}', {}, {}, {}", randomString, (void *)&randomString,
       512, 3.14159);

  int a = 4;
  auto sptr = std::make_shared<int>(5);
  auto uptr = std::make_unique<int>(6);
  logi("void ptr: {}, ptr: {}, sptr: {}, uptr: {}", (void *)&a, &a, sptr, std::move(uptr));
  a = 7;
  *sptr = 8;

  char strarr[10] = "111";
  char *cstr = strarr;
  std::string str = "aaa";
  logi("str: {}, pstr: {}, strarr: {}, pstrarr: {}, cstr: {}, pcstr: {}", str, &str, strarr, &strarr, cstr, &cstr);
  str = "bbb";
  strcpy(cstr, "222");

  // logi(FMT_STRING("This msg will trigger compile error: {:d}"), "I am not a number");
  // FMT_STRING() above is not needed for c++20

  logd("This message wont be logged since it is lower "
       "than the current log level.");
  fmtlog::setLogLevel(fmtlog::DBG);
  logd("Now debug msg is shown");

  fmtlog::poll();

  for (int i = 0; i < 3; i++)
  {
    logio("log once: {}", i);
  }

  fmtlog::setThreadName("main");
  logi("Thread name changed");

  fmtlog::poll();

  fmtlog::setHeaderPattern("{YmdHMSF} {s} {l}[{t}] ");
  logi("Header pattern is changed, full date time info is shown");

  fmtlog::poll();

  logi("VG custom types: {} end", Foo(10, 34.56));
  fmtlog::poll();

  VgBuffer1024 vgb;
  vgb.load(Foo(520, 524.56));

// std::cout << "vgb=" << vgb << std::endl;
// std::cout << "VG2 useDataSize=" << has_member(VgBuffer1024, useDataSize) << std::endl;

  logi("VG2 custom types: {} end", vgb);
  fmtlog::poll();

  vgb.useDataSize_=false;
  logi("VG3 custom types: {} end", vgb);
  fmtlog::poll();

  logi("test custom types: {}, {}, {}", MyType(1), MyType(2), MovableType(3));
  fmtlog::poll();


  for (int i = 0; i < 10; i++)
  {
    logil(10, "This msg will be logged at an interval of at least 10 ns: {}.", i);
  }

  fmtlog::poll();

  fmtlog::setLogCB(logcb, fmtlog::WRN);
  logw("This msg will be called back");

  fmtlog::setLogFile("/tmp/wow", false);
  logw("VG1_W custom types: {} end", Foo(120, 234.56));
  fmtlog::poll();


  for (int i = 0; i < 10; i++)
  {
    logw("test logfilepos: {}.", i);
  }

  // fmtlog::setLogQFullCB(logQFullCB, nullptr);
  // for (int i = 0; i < 1024; i++)
  // {
  //   std::string str(1000, ' ');
  //   logi("log q full cb test: {}", str);
  // }

  fmtlog::poll();
  // runBenchmark();
  // runBenchmark1();

  return 0;
}

void runBenchmark()
{
  const int RECORDS = 10000;
  // fmtlog::setLogFile("/dev/null", false);
  fmtlog::closeLogFile();
  fmtlog::setLogCB(nullptr, fmtlog::WRN);

  std::chrono::high_resolution_clock::time_point t0, t1;

  t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < RECORDS; ++i)
  {
    logi("Simple log message with one parameters, {}", i);
  }
  t1 = std::chrono::high_resolution_clock::now();

  double span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0).count();
  fmt::print("benchmark, front latency: {:.1f} ns/msg average\n", (span / RECORDS) * 1e9);
}

void runBenchmark1()
{
  const int RECORDS = 10000;
  fmtlog::setLogFile("/tmp/vg1", true);

  std::chrono::high_resolution_clock::time_point t0, t1;

  t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < RECORDS; ++i)
  {
    logi("Foo={}", Foo(i, i + 0.234));
  }
  t1 = std::chrono::high_resolution_clock::now();

  double span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0).count();
  fmtlog::poll();
  fmtlog::closeLogFile();

  fmt::print("benchmark, front latency Foo: {:.1f} ns/msg average\n", (span / RECORDS) * 1e9);
}
