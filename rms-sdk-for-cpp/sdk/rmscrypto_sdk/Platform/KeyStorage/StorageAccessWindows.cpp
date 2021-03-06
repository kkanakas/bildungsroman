#include "StorageAccessWindows.h"
#include <strsafe.h>
#include "../../CryptoAPI/RMSCryptoExceptions.h"

using namespace rmscrypto::exceptions;

// Global static pointer used to ensure a single instance of the class.
StorageAccessWindows* StorageAccessWindows::mInstance = NULL;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/

StorageAccessWindows* StorageAccessWindows::Instance()
{
   if (!mInstance)   // Only allow one instance of class to be generated.
      mInstance = new StorageAccessWindows;

   return mInstance;
}

LPWSTR ConcatWStrings(LPCWSTR stringA, LPCWSTR stringB)
{
    DWORD lengthA = wcslen(stringA);
    DWORD lengthB = wcslen(stringB);
    LPWSTR result = new WCHAR[lengthA + lengthB + 1];
    StringCchCopy(result, lengthA + 1, stringA);
    StringCchCat(result, lengthA + lengthB + 1, stringB);
    return result;
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}

StorageAccessWindows::StorageAccessWindows()
{
    LPCWSTR directory = CreateLocalStorage();
    LPCWSTR dbName = ConcatWStrings(directory, L"\\MSIPCKeyStorage.db");

    char * error;
    int rc = sqlite3_open16(dbName, &mDb);
    const char *sqlCreateTable = "CREATE TABLE IF NOT EXISTS MSIPCKeyStorage (csKeyWrapper STRING PRIMARY KEY, csKey STRING);";
    rc = sqlite3_exec(mDb, sqlCreateTable, NULL, NULL, &error);
    if (rc)
    {
        // error can be used for telemetry
        sqlite3_free(error);
        throw RMSCryptoIOKeyException(sqlite3_errmsg(mDb));
    }
}

LPWSTR StorageAccessWindows::CreateLocalStorage()
{
    //Fetch environment variable %localappdata%
    //Check if db folder exists
    //If not, create and encrypt

    LPCWSTR lpName = L"localappdata";
    LPCWSTR aipDir = L"\\Microsoft\\AIP";
    DWORD nSize = 500;
    LPWSTR lpBuffer = new WCHAR[nSize];
    GetEnvironmentVariableW(lpName, lpBuffer, nSize);
    LPWSTR directory = ConcatWStrings(lpBuffer, aipDir);
    if (!CreateDirectory(directory, NULL))
    {
        DWORD lastError = GetLastError();
        if (lastError != ERROR_ALREADY_EXISTS)
        {
            throw RMSCryptoIOKeyException(GetLastErrorAsString().c_str());
        }
    }
    else
    {
        if (!EncryptFile(directory))
        {
            throw RMSCryptoIOKeyException(GetLastErrorAsString().c_str());
        }
    }
    return directory;
}

void StorageAccessWindows::StoreKey(const std::string& csKeyWrapper,
                                    const std::string& csKey)
{
    char * error;
    std::string sqlInsert = "INSERT INTO MSIPCKeyStorage VALUES('"+csKeyWrapper +"','"+csKey+"');";
    int rc = sqlite3_exec(mDb, sqlInsert.c_str(), NULL, NULL, &error);
    if (rc)
    {
        // error can be used for telemetry
        sqlite3_free(error);
        throw RMSCryptoIOKeyException(sqlite3_errmsg(mDb));
    }
}

std::shared_ptr<std::string> StorageAccessWindows::LookupKey(const std::string& csKeyWrapper)
{
    char * error;
    std::string sqlLookup = "SELECT csKey FROM MSIPCKeyStorage WHERE csKeyWrapper ='" + csKeyWrapper + "';";
    char **results = NULL;
    int rows, columns;
    int rc = sqlite3_get_table(mDb, sqlLookup.c_str(), &results, &rows, &columns, &error);
    if (rc)
    {
        // error can be used for telemetry
        sqlite3_free(error);
        throw RMSCryptoIOKeyException(sqlite3_errmsg(mDb));
    }
    return rows >=1 && columns >= 1 ? std::shared_ptr<std::string>(new std::string(results[1])) : nullptr;
}

void StorageAccessWindows::RemoveKey(const std::string& csKeyWrapper)
{
    char * error;
    std::string sqlDelete = "DELETE FROM MSIPCKeyStorage WHERE csKeyWrapper = '" + csKeyWrapper +"';";
    int rc = sqlite3_exec(mDb, sqlDelete.c_str(), NULL, NULL, &error);
    if (rc)
    {
        // error can be used for telemetry
        sqlite3_free(error);
        throw RMSCryptoIOKeyException(sqlite3_errmsg(mDb));
    }
}
