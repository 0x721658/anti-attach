
#ifndef ATTACH_HXX
#define ATTACH_HXX

#include <memory>
#include <windows.h>
#include <string>
#include <winternl.h>
#include <iostream>

class already_debugged
{
public:
	void dispatch( int argc, char* argv[ ] ) const
	{
		argc == 1 ? this->main_process( ) : this->child_process( std::atoi( argv[ 1 ] ) );
	}

private:
	void child_process( const unsigned long parent_pid ) const
	{
		void* parent_handle = OpenProcess( PROCESS_ALL_ACCESS, 0, parent_pid );
		if ( parent_handle == nullptr or DebugActiveProcess( parent_pid ) == 0 )
		{
			CloseHandle( parent_handle );
			return;
		}

		void* debug_handle = *( void** ) ( ( unsigned char* ) __readgsqword( 0x30 ) + 0x16a8 );
		if ( !debug_handle )
		{
			CloseHandle( parent_handle );
			return;
		}

		void* cloned_handle = nullptr;
		if ( !DuplicateHandle( GetCurrentProcess( ), debug_handle, parent_handle, &cloned_handle, 0, 0, DUPLICATE_SAME_ACCESS ) )
		{
			CloseHandle( parent_handle );
			return;
		}

		DEBUG_EVENT event;
		for ( unsigned long timeouts = 0; timeouts < 10;)
		{
			if ( !WaitForDebugEvent( &event, 100 ) )
			{
				++timeouts;
				continue;
			}

			timeouts = 0;
			ContinueDebugEvent( event.dwProcessId, event.dwThreadId, DBG_CONTINUE );
		}

		CloseHandle( cloned_handle );
		CloseHandle( debug_handle );
		CloseHandle( parent_handle );
		ExitThread( 0 );
	}

	void main_process( ) const
	{
		char process_path[ MAX_PATH ];
		char command_line[ MAX_PATH + 32 ];

		{
			GetModuleFileNameA( nullptr, process_path, sizeof( process_path ) );
			std::snprintf( command_line, sizeof( command_line ), "\"%s\" %lu", process_path, GetCurrentProcessId( ) );
		}

		STARTUPINFOA startup = { sizeof( STARTUPINFOA ) };
		PROCESS_INFORMATION process = {};

		if ( CreateProcessA( nullptr, command_line, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup, &process ) )
		{
			CloseHandle( process.hProcess );
			CloseHandle( process.hThread );
			WaitForSingleObject( process.hProcess, INFINITE );
		}
	}
};
inline const auto c_already_debugged = std::make_unique<already_debugged>( );

#endif // !ATTACH_HXX
