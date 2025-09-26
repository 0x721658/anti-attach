simple trick, self debugs your own process preventing debuggers like x64dbg from attaching

## how to implement
1. include attachment.hxx
2. call the "start" function at the very start of your program, example below:
```cpp
int main(int argc, char* argv[])
{
	c_already_debugged->start(argc, argv);

	std::printf("attachment disabled\n");
	std::cin.get();

	return 0;
}
```


