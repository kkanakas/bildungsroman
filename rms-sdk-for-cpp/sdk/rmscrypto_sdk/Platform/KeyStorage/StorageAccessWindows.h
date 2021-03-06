#ifndef STORAGEACCESSWINDOWS_H
#define STORAGEACCESSWINDOWS_H
#include "sqlite3.h"
#include <string>
#include <memory>
#include <Windows.h>

class StorageAccessWindows{
public:
   static StorageAccessWindows* Instance();
   void StoreKey(const std::string&, const std::string&);
   void RemoveKey(const std::string&);
   std::shared_ptr<std::string> LookupKey(const std::string&);

private:
   StorageAccessWindows();  // Private so that it can  not be called
   StorageAccessWindows(StorageAccessWindows const&){}             // copy constructor is private
   StorageAccessWindows& operator=(StorageAccessWindows const&){}  // assignment operator is private
   static StorageAccessWindows* mInstance;
   sqlite3 * mDb;
   LPWSTR CreateLocalStorage();
};

#endif // STORAGEACCESSWINDOWS_H
