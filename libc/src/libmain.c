
// Main should be declared in application
extern int main();

// C code entry point
int __g_main()
{
	return main();	
}

// Called at program exit
int __g_exit()
{

}

