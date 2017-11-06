#ifndef GUARD_API_REGISTRY_HPP
#define GUARD_API_REGISTRY_HPP

#include <unordered_map>
#include <string>

/*

  Registry is a global, canonical, table of apis, indexed through their type.

  The registry may be included from any compilation unit / .cpp file,
  in order to get hold of the current function pointers to api methods.

  Indexing by type works via lookup of a `static const char* id`
  field, which each api must provide. As this is effectively
  an immutable pointer to a string literal, the pointer address
  will be available for the duration of the program, and it is assumed
  to be unique.

*/

struct pal_api_loader_i;
struct pal_api_loader_o;

class Registry {
	static std::unordered_map<const char *, void *> apiTable;

	template <typename T>
	inline static constexpr auto getPointerToStaticRegFun() noexcept {
		return T::pRegFun;
	}

	static bool loaderCallback( void * );

	struct CallbackParams {
		pal_api_loader_i *loaderInterface;
		pal_api_loader_o *loader;
		void *            api;
		const char *      lib_register_fun_name;
	};

	// We need these loader-related methods so we can keep this header file opaque,
	// i.e. we don't want to include ApiLoader.h in this header file - as this header
	// file will get included by lots of other headers.
	//
	// This is messy, i know, but since the templated method addApiDynamic must live
	// in the header file, these methods must be declared here, too.

	static pal_api_loader_i *getLoaderInterface();
	static pal_api_loader_o *createLoader( pal_api_loader_i *loaderInterface, const char *libPath_ );
	static void              loadLibrary( pal_api_loader_i *loaderInterface, pal_api_loader_o *loader );
	static void              registerApi( pal_api_loader_i *loaderInterface, pal_api_loader_o *loader, void *api, const char *api_register_fun_name );

	static int addWatch( const char *watchedPath, CallbackParams &settings );

public:
	template <typename T>
	inline static constexpr auto getId() noexcept {
		return T::id;
	}

	template <typename T>
	static T *addApiStatic() {
		static auto api = getApi<T>();
		// We assume failed map lookup returns a pointer which is
		// initialised to be a nullptr.
		if ( api == nullptr ) {
			api = new T();
			( *getPointerToStaticRegFun<T>() )( api );
			apiTable[ getId<T>() ] = api;
		}
		return api;
	}


	template <typename T>
	static T *addApiDynamic( bool shouldWatchForAutoReload = false ) {

		// Because this is a templated function, there will be
		// static memory allocated for each type this function will get
		// fleshed out with.
		// We want this, as we use the addresses of these static variables
		// for the life-time of the application.

		static auto apiName = getId<T>();
		static auto api     = getApi<T>();

		if ( api == nullptr ) {

			static const std::string lib_path              = "./" + std::string{apiName} + "/lib" + std::string{apiName} + ".so";
			static const std::string lib_register_fun_name = "register_" + std::string{apiName} + "_api";

			static pal_api_loader_i *loaderInterface = getLoaderInterface();
			static pal_api_loader_o *loader          = createLoader( loaderInterface, lib_path.c_str() );

			api = new T();
			loadLibrary( loaderInterface, loader );
			registerApi( loaderInterface, loader, api, lib_register_fun_name.c_str() );

			apiTable[ getId<T>() ] = api;

			// ----
			if ( shouldWatchForAutoReload ) {
				static CallbackParams callbackParams = {loaderInterface, loader, api, lib_register_fun_name.c_str()};
				// TODO: We keep watchId static so that a watch is only created once per type T.
				// ideally, if we ever wanted to be able to remove watches, we'd keep the watchIds in a
				// table, similar to the apiTable.
				static int            watchId        = addWatch( lib_path.c_str(), callbackParams );
			}

		} else {
			// todo: we should warn that this api was already added.
		}

		return api;
	}

	template <typename T>
	static T *getApi() {
		// WARNING: this will return a void* if nothing found!
		// TODO: add error checking if compiled in debug mode.
		return static_cast<T *>( apiTable[ getId<T>() ] );
	}

	static void pollForDynamicReload();
};

#endif // GUARD_API_REGISTRY_HPP
