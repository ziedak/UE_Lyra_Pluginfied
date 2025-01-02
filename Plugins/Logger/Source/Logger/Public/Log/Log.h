#pragma once

#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"

LOGGER_API DECLARE_LOG_CATEGORY_EXTERN(LogGAS, Log, All);

LOGGER_API DECLARE_LOG_CATEGORY_EXTERN(LogCORE, Log, All);

LOGGER_API DECLARE_LOG_CATEGORY_EXTERN(LogExperience, Log, All);

//CUSTOMCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogGASAbilitySystem, Log, All);
//CUSTOMCORE_API DECLARE_LOG_CATEGORY_EXTERN(LogGASTeams, Log, All);

#ifdef _MSC_VER
#define __CLASS_FUNCTION__ __FUNCTION__
#endif

#ifdef __GNUG__
#include <cxxabi.h>
#include <execinfo.h>
char* class_func(const char* c, const char* f)
{
	int status;
	static char buff[100];
	char* demangled = abi::__cxa_demangle(c, NULL, NULL, &status);
	snprintf(buff, sizeof(buff), "%s::%s", demangled, f);
	free(demangled);
	return buff;
}
#define __CLASS_FUNCTION__ class_func(typeid(*this).name(), __func__)
#endif

 LOGGER_API FString GetClientServerContext(UObject* ContextObject);

void PrintCallStack();

void ConsoleLog(const FString& Message);

/* ClassName::FunctionName where this is called. */
#define CURRENT_CLASS_FUNCTION (FString(__CLASS_FUNCTION__))
/* Class Name where this is called. */
#define CURRENT_CLASS (FString(__CLASS_FUNCTION__).Left(FString(__CLASS_FUNCTION__).Find(TEXT(":"))) )
/* Function Name where this is called. */
#define CURRENT_FUNCTION (FString(__CLASS_FUNCTION__).Right(FString(__CLASS_FUNCTION__).Len() - FString(__CLASS_FUNCTION__).Find(TEXT("::")) - 2 ))
/* Line Number where this is called. */
#define CURRENT_LINE  (FString::FromInt(__LINE__))
/* Class Name and Line Number where this is called. */
#define CURRENT_CLASS_LINE (CURRENT_CLASS + "(" + CURRENT_LINE + ")")
/* Class Name and Function Name and Line Number where this is called. */
#define CURRENT_CLASS_FUNCTION_LINE (CURRENT_CLASS_FUNCTION + ":" + CURRENT_LINE )
/* Function Signature where this is called. */
#define CURRENT_FUNCTIONSIG (FString(__FUNCSIG__))

#define CLIENT_SERVER_CONTEXT     GetClientServerContext(this)
#define OBJECT_OWNER  GetNameSafe(GetOwner())
#define CLASS_OWNER   GetNameSafe(this)

// TODO SHow log by condition

/*
 * UE_LOG macros
 */
#define LOG_SIMPLE2(CategoryName,Verbosity, Message) UE_LOG(CategoryName, Verbosity,TEXT("[%s] [%s -> %s] %s : %s"), *CLIENT_SERVER_CONTEXT, *OBJECT_OWNER, *CLASS_OWNER, *CURRENT_CLASS_FUNCTION_LINE, *FString(Message))

#define LOG_GLOBAL_SIMPLE(Condition,CategoryName,Verbosity, Message)  \
	{ \
		if (Condition) { \
			LOG_SIMPLE2(CategoryName, Verbosity, Message); \
		} \
	}

#if WITH_EDITOR
/* Log macro. Style: ClassName::FunctionName (Line) Message. */
#define LOG(CategoryName,Verbosity, FormatString , ...) UE_LOG(CategoryName, Verbosity, TEXT("[%s] [%s -> %s] %s : %s"), *CLIENT_SERVER_CONTEXT, *OBJECT_OWNER, *CLASS_OWNER, *CURRENT_CLASS_FUNCTION_LINE, *FString::Printf(TEXT(FormatString), ##__VA_ARGS__ ) )
#else
	#define LOG(CategoryName, Verbosity, FormatString, ...) {}
#endif
/* Log warning macro. Style: ClassName::FunctionName (Line) Message. */


#define UELOG(CategoryName,Verbosity, FormatString , ...) UE_LOG(CategoryName, Verbosity, TEXT("%s : %s"),  *CURRENT_CLASS_FUNCTION_LINE, *FString::Printf(TEXT(FormatString), ##__VA_ARGS__ ) )
#define ULOG_INFO(CategoryName, FormatString , ...) UELOG(CategoryName, Log, FormatString,  ##__VA_ARGS__ )
#define ULOG_WARNING(CategoryName, FormatString , ...) UELOG(CategoryName, Warning, FormatString,  ##__VA_ARGS__ )
#define ULOG_ERROR(CategoryName, FormatString , ...) UELOG(CategoryName, Error, FormatString, ##__VA_ARGS__ )
#define ULOG_FATAL(CategoryName, FormatString , ...) UELOG(CategoryName,Fatal,FormatString, ##__VA_ARGS__ )


#define LOG_INFO(CategoryName, FormatString , ...) LOG(CategoryName, Log, FormatString,  ##__VA_ARGS__ )
#define LOG_WARNING(CategoryName, FormatString , ...) LOG(CategoryName, Warning, FormatString,  ##__VA_ARGS__ )
#define LOG_ERROR(CategoryName, FormatString , ...) LOG(CategoryName, Error, FormatString, ##__VA_ARGS__ )
#define LOG_FATAL(CategoryName, FormatString , ...) LOG(CategoryName,Fatal,FormatString, ##__VA_ARGS__ )

/*
 * Screen Messages macros
 */
#if WITH_EDITOR
#define LOG_SCREEN(Color, FormatString , ...) if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.0f, Color, *(CURRENT_CLASS_FUNCTION_LINE + ": [ INFO ] " + (FString::Printf(TEXT(FormatString), ##__VA_ARGS__ ))))
#else
	#define LOG_SCREEN(Color, Message) {}
#endif

#define LOG_SCREEN_INFO(FormatString , ...) LOG_SCREEN(FColor::Cyan, FormatString , ##__VA_ARGS__)
#define LOG_SCREEN_WARNING(FormatString , ...) LOG_SCREEN(FColor::Yellow, FormatString , ##__VA_ARGS__)
#define LOG_SCREEN_ERROR(FormatString , ...) LOG_SCREEN(FColor::Red, FormatString , ##__VA_ARGS__)
