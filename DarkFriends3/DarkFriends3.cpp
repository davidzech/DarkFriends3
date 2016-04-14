// DarkFriends3.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Log.h"
#include "steam/steam_api.h"

template<class T>
T* GetSteamInterface(const char* name) {
	static HMODULE steamHandle = nullptr;
	if (steamHandle == nullptr) {
		steamHandle = GetModuleHandle(TEXT("steam_api64.dll"));
		if (steamHandle == nullptr) {
			steamHandle = LoadLibrary(TEXT("steam_api64.dll"));
		}
	}

	T* addr =  reinterpret_cast<T*>(GetProcAddress(steamHandle, name)());
	if (addr == nullptr) {
		Log("Couldn't resolve Steam Interface %s", name);
	}
	return addr;
}

typedef bool(__fastcall *PSendP2PPacket)(ISteamNetworking *pThis, CSteamID steamIDRemote, const void * pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel);
typedef bool(__fastcall *PReadP2PPacket)(ISteamNetworking *pThis, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel);
typedef bool(__fastcall *PAcceptP2PSessionWithUser)(CSteamID steamIDRemote);
PSendP2PPacket OSendP2PPacket = nullptr;
PReadP2PPacket OReadP2PPacket = nullptr;
PAcceptP2PSessionWithUser OAcceptP2PSessionWithUser = nullptr;

bool __fastcall SendP2PPacket(ISteamNetworking *pThis,
	CSteamID steamIDRemote, const void * pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel) {

	Log("SendP2PPacket called for remote ID: %llu", steamIDRemote.ConvertToUint64());

	ISteamFriends* friends = GetSteamInterface<ISteamFriends>("SteamFriends");
	if (friends->GetFriendRelationship(steamIDRemote) == k_EFriendRelationshipFriend) {
		return OSendP2PPacket(pThis, steamIDRemote, pubData, cubData, eP2PSendType, nChannel);
	}
	else {
		return false;
	}
}

bool __fastcall ReadP2PPacket(ISteamNetworking *pThis, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel) {

	Log("ReadP2PPacket called for remote ID: %llu", psteamIDRemote->ConvertToUint64());

	OReadP2PPacket(pThis, pubDest, cubDest, pcubMsgSize, psteamIDRemote, nChannel);
	ISteamFriends* friends = GetSteamInterface<ISteamFriends>("SteamFriends");
	return friends->GetFriendRelationship(*psteamIDRemote) == k_EFriendRelationshipFriend;
}

bool __fastcall AcceptP2PSessionWithUser(CSteamID steamIDRemote) {
	ISteamFriends* friends = GetSteamInterface<ISteamFriends>("SteamFriends");
	if (friends->GetFriendRelationship(steamIDRemote) == k_EFriendRelationshipFriend) {
		Log("Accepted P2P session request from friend: %llu", steamIDRemote.ConvertToUint64());
		return OAcceptP2PSessionWithUser(steamIDRemote);
	}
	else {
		Log("Rejected P2P session request from stranger: %llu", steamIDRemote.ConvertToUint64());
		return false;
	}
}


void Bootstrap(BYTE* debug) {
	if (*debug) {
		CreateDebugConsole();
	}

	Log("DarkFriends3 attached to process successfully");

	ISteamNetworking* networking = GetSteamInterface<ISteamNetworking>("SteamNetworking");	
	
	Log("Found ISteamNetworking at %016X", networking);

	Log("Searching for VTable...");

	DWORD64* pVTable = (DWORD64*)*(DWORD64*)networking;
	if (pVTable != nullptr) {
		Log("Found VTable at %016X", pVTable);
		
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery((LPCVOID)pVTable, &mbi, sizeof(mbi));
		VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect);

		Log("Marked memory as READ | WRITE");
		
		OSendP2PPacket = (PSendP2PPacket)pVTable[0];
		OReadP2PPacket = (PReadP2PPacket)pVTable[2];

		Log("Backed up original P2P Functions");

		pVTable[0] = (DWORD64)SendP2PPacket;
		pVTable[2] = (DWORD64)ReadP2PPacket;
		pVTable[3] = (DWORD64)AcceptP2PSessionWithUser;

		Log("Replaced P2P Functions");

		VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);

		Log("Memory reprotected");

		Log("Done hooking functions");
	}
	else {
		Log("Couldn't find ISteamNetworking VTable");
	}
}