﻿#include "framework.h"
#include <sstream>
#include <vector>

#include "exec_sql.h"
#include "log.h"
#include "send_msg.h"
#include "spy_types.h"
#include "util.h"

extern HANDLE g_hEvent;
extern WxCalls_t g_WxCalls;
extern QWORD g_WeChatWinDllAddr;
extern string GetSelfWxid(); // Defined in spy.cpp

typedef QWORD (*funcNew_t)(QWORD);
typedef QWORD (*funcFree_t)(QWORD);
typedef QWORD (*funcSendMsgMgr_t)();
typedef QWORD (*funcGetAppMsgMgr_t)();

typedef QWORD (*funcSendTextMsg_t)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD, QWORD);
typedef QWORD (*funcSendImageMsg_t)(QWORD, QWORD, QWORD, QWORD, QWORD);
typedef QWORD (*funcSendFileMsg_t)(QWORD, QWORD, QWORD, QWORD, QWORD, QWORD *, QWORD, QWORD *, QWORD, QWORD *, QWORD,
                                   QWORD);
typedef QWORD (*funcSendRichTextMsg_t)(QWORD, QWORD, QWORD);
typedef QWORD (*funcSendPatMsg_t)(QWORD, QWORD);
typedef QWORD (*funcForwardMsg_t)(QWORD, QWORD, QWORD, QWORD);

void SendTextMessage(string wxid, string msg, string atWxids)
{
    QWORD success  = 0;
    wstring wsWxid = String2Wstring(wxid);
    wstring wsMsg  = String2Wstring(msg);
    WxString wxMsg(wsMsg);
    WxString wxWxid(wsWxid);

    vector<wstring> vAtWxids;
    vector<WxString> vWxAtWxids;
    if (!atWxids.empty()) {
        wstringstream wss(String2Wstring(atWxids));
        while (wss.good()) {
            wstring wstr;
            getline(wss, wstr, L',');
            vAtWxids.push_back(wstr);
            WxString wxAtWxid(vAtWxids.back());
            vWxAtWxids.push_back(wxAtWxid);
        }
    } else {
        WxString wxEmpty = WxString();
        vWxAtWxids.push_back(wxEmpty);
    }

    QWORD wxAters = (QWORD) & ((RawVector_t *)&vWxAtWxids)->start;

    char buffer[0x460]                = { 0 };
    funcSendMsgMgr_t funcSendMsgMgr   = (funcSendMsgMgr_t)(g_WeChatWinDllAddr + g_WxCalls.sendText.call1);
    funcSendTextMsg_t funcSendTextMsg = (funcSendTextMsg_t)(g_WeChatWinDllAddr + g_WxCalls.sendText.call2);
    funcFree_t funcFree               = (funcFree_t)(g_WeChatWinDllAddr + g_WxCalls.sendText.call3);
    funcSendMsgMgr();
    success = funcSendTextMsg((QWORD)(&buffer), (QWORD)(&wxWxid), (QWORD)(&wxMsg), wxAters, 1, 1, 0, 0);
    funcFree((QWORD)(&buffer));
}

void SendImageMessage(string wxid, string path)
{
    wstring wsWxid = String2Wstring(wxid);
    wstring wsPath = String2Wstring(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);

    funcNew_t funcNew                = (funcNew_t)(g_WeChatWinDllAddr + g_WxCalls.sendImg.call1);
    funcFree_t funcFree              = (funcFree_t)(g_WeChatWinDllAddr + g_WxCalls.sendImg.call2);
    funcSendMsgMgr_t funcSendMsgMgr  = (funcSendMsgMgr_t)(g_WeChatWinDllAddr + g_WxCalls.sendImg.call3);
    funcSendImageMsg_t funcSendImage = (funcSendImageMsg_t)(g_WeChatWinDllAddr + g_WxCalls.sendImg.call4);

    char msg[0x460]    = { 0 };
    char msgTmp[0x460] = { 0 };
    QWORD *flag[10]    = { 0 };

    QWORD tmp1 = 0, tmp2 = 0;
    QWORD pMsgTmp = funcNew((QWORD)(&msgTmp));
    flag[8]       = &tmp1;
    flag[9]       = &tmp2;
    flag[1]       = (QWORD *)(pMsgTmp);

    QWORD pMsg    = funcNew((QWORD)(&msg));
    QWORD sendMgr = funcSendMsgMgr();
    funcSendImage(sendMgr, pMsg, (QWORD)(&wxWxid), (QWORD)(&wxPath), (QWORD)(&flag));
    funcFree(pMsg);
    funcFree(pMsgTmp);
}

void SendFileMessage(string wxid, string path)
{
    wstring wsWxid = String2Wstring(wxid);
    wstring wsPath = String2Wstring(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);

    funcNew_t funcNew                   = (funcNew_t)(g_WeChatWinDllAddr + g_WxCalls.sendFile.call1);
    funcFree_t funcFree                 = (funcFree_t)(g_WeChatWinDllAddr + g_WxCalls.sendFile.call2);
    funcGetAppMsgMgr_t funcGetAppMsgMgr = (funcGetAppMsgMgr_t)(g_WeChatWinDllAddr + g_WxCalls.sendFile.call3);
    funcSendFileMsg_t funcSendFile      = (funcSendFileMsg_t)(g_WeChatWinDllAddr + g_WxCalls.sendFile.call4);

    char msg[0x460] = { 0 };
    QWORD tmp1[4]   = { 0 };
    QWORD tmp2[4]   = { 0 };
    QWORD tmp3[4]   = { 0 };

    QWORD pMsg   = funcNew((QWORD)(&msg));
    QWORD appMgr = funcGetAppMsgMgr();
    funcSendFile(appMgr, pMsg, (QWORD)(&wxWxid), (QWORD)(&wxPath), 1, tmp1, 0, tmp2, 0, tmp3, 0, 0);
    funcFree(pMsg);
}

int SendRichTextMessage(RichText_t &rt)
{ // TODO: Fix memory leak
#define SRTM_SIZE 0x3F0
    QWORD status = -1;

    funcNew_t funcNew                          = (funcNew_t)(g_WeChatWinDllAddr + g_WxCalls.rt.call1);
    funcFree_t funcFree                        = (funcFree_t)(g_WeChatWinDllAddr + g_WxCalls.rt.call2);
    funcGetAppMsgMgr_t funcGetAppMsgMgr        = (funcGetAppMsgMgr_t)(g_WeChatWinDllAddr + g_WxCalls.rt.call3);
    funcSendRichTextMsg_t funcForwordPublicMsg = (funcSendRichTextMsg_t)(g_WeChatWinDllAddr + g_WxCalls.rt.call4);

    char *buff = (char *)HeapAlloc(GetProcessHeap(), 0, SRTM_SIZE);
    if (buff == NULL) {
        LOG_ERROR("Out of Memory...");
        return -1;
    }

    memset(buff, 0, SRTM_SIZE);
    funcNew((QWORD)buff);
    WxString *pReceiver = NewWxStringFromStr(rt.receiver);
    WxString *pTitle    = NewWxStringFromStr(rt.title);
    WxString *pUrl      = NewWxStringFromStr(rt.url);
    WxString *pThumburl = NewWxStringFromStr(rt.thumburl);
    WxString *pDigest   = NewWxStringFromStr(rt.digest);
    WxString *pAccount  = NewWxStringFromStr(rt.account);
    WxString *pName     = NewWxStringFromStr(rt.name);

    memcpy(buff + 0x8, pTitle, sizeof(WxString));
    memcpy(buff + 0x48, pUrl, sizeof(WxString));
    memcpy(buff + 0xB0, pThumburl, sizeof(WxString));
    memcpy(buff + 0xF0, pDigest, sizeof(WxString));
    memcpy(buff + 0x2C0, pAccount, sizeof(WxString));
    memcpy(buff + 0x2E0, pName, sizeof(WxString));

    QWORD mgr = funcGetAppMsgMgr();
    status    = funcForwordPublicMsg(mgr, (QWORD)(pReceiver), (QWORD)(buff));
    funcFree((QWORD)buff);

    return (int)status;
}

int SendPatMessage(string roomid, string wxid)
{
    QWORD status = -1;

    wstring wsRoomid = String2Wstring(roomid);
    wstring wsWxid   = String2Wstring(wxid);
    WxString wxRoomid(wsRoomid);
    WxString wxWxid(wsWxid);

    funcSendPatMsg_t funcSendPatMsg = (funcSendPatMsg_t)(g_WeChatWinDllAddr + g_WxCalls.pm.call1);

    status = funcSendPatMsg((QWORD)(&wxRoomid), (QWORD)(&wxWxid));
    return (int)status;
}

int ForwardMessage(QWORD msgid, string receiver)
{
    int status     = -1;
    uint32_t dbIdx = 0;
    QWORD localId  = 0;

    funcForwardMsg_t funcForwardMsg = (funcForwardMsg_t)(g_WeChatWinDllAddr + g_WxCalls.fm.call1);
    if (GetLocalIdandDbidx(msgid, &localId, &dbIdx) != 0) {
        LOG_ERROR("Failed to get localId, Please check id: {}", to_string(msgid));
        return status;
    }

    WxString *pReceiver = NewWxStringFromStr(receiver);

    LARGE_INTEGER l;
    l.HighPart = dbIdx;
    l.LowPart  = (DWORD)localId;

    status = (int)funcForwardMsg((QWORD)pReceiver, l.QuadPart, 0x4, 0x0);

    return status;
}

#if 0
void SendXmlMessage(string receiver, string xml, string path, int type)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }

    // 发送消息Call地址 = 微信基址 + 偏移
    DWORD sendXmlCall1 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call1;
    DWORD sendXmlCall2 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call2;
    DWORD sendXmlCall3 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call3;
    DWORD sendXmlCall4 = g_WeChatWinDllAddr + g_WxCalls.sendXml.call4;
    DWORD sendXmlParam = g_WeChatWinDllAddr + g_WxCalls.sendXml.param;

    char buffer[0xFF0] = { 0 };
    char nullBuf[0x1C] = { 0 };

    wstring wsSender   = String2Wstring(GetSelfWxid());
    wstring wsReceiver = String2Wstring(receiver);
    wstring wsXml      = String2Wstring(xml);

    WxString wxPath;
    WxString wxNull;
    WxString wxXml(wsXml);
    WxString wxSender(wsSender);
    WxString wxReceiver(wsReceiver);

    if (!path.empty()) {
        wstring wsPath = String2Wstring(path);
        wxPath         = WxString(wsPath);
    }

    DWORD sendtype = type;
    __asm {
		pushad;
		pushfd;
		lea ecx, buffer;
		call sendXmlCall1;
		mov eax, [sendtype];
		push eax;
		lea eax, nullBuf;
		lea edx, wxSender;
		push eax;
		lea eax, wxPath;
		push eax;
		lea eax, wxXml;
		push eax;
		lea edi, wxReceiver;
		push edi;
		lea ecx, buffer;
		call sendXmlCall2;
		add esp, 0x14;
		lea eax, wxNull;
		push eax;
		lea ecx, buffer;
		call sendXmlCall3;
		mov dl, 0x0;
		lea ecx, buffer;
		push sendXmlParam;
		push sendXmlParam;
		call sendXmlCall4;
		add esp, 0x8;
		popfd;
		popad;
    }
}

void SendEmotionMessage(string wxid, string path)
{
    if (g_WeChatWinDllAddr == 0) {
        return;
    }

    char buffer[0x1C] = { 0 };
    wstring wsWxid    = String2Wstring(wxid);
    wstring wsPath    = String2Wstring(path);

    WxString wxWxid(wsWxid);
    WxString wxPath(wsPath);
    WxString nullbuffer;

    // 发送文件Call地址 = 微信基址 + 偏移
    DWORD sendCall1 = g_WeChatWinDllAddr + g_WxCalls.sendEmo.call1;
    DWORD sendCall2 = g_WeChatWinDllAddr + g_WxCalls.sendEmo.call2;
    DWORD sendCall3 = g_WeChatWinDllAddr + g_WxCalls.sendEmo.call3;

    __asm {
        pushad;
        pushfd;
        mov ebx, dword ptr[sendCall3];
        lea eax, buffer;
        push eax;
        push 0x0;
        sub esp, 0x14;
        mov esi, esp;
        mov dword ptr [esi], 0x0;
        mov dword ptr [esi+0x4], 0x0;
        mov dword ptr [esi+0x8], 0x0;
        mov dword ptr [esi+0xC], 0x0;
        mov dword ptr [esi+0x10], 0x0;
        push 0x2;
        lea eax, wxWxid;
        sub esp, 0x14;
        mov ecx, esp;
        push eax;
        call sendCall1;
        sub esp, 0x14;
        mov esi, esp;
        mov dword ptr [esi], 0x0;
        mov dword ptr [esi+0x4], 0x0;
        mov dword ptr [esi+0x8], 0x0;
        mov dword ptr [esi+0xC], 0x0;
        mov dword ptr [esi+0x10], 0x0;
        sub esp, 0x14;
        mov ecx, esp;
        lea eax, wxPath;
        push eax;
        call sendCall1;
        mov ecx, ebx;
        call sendCall2;
        popfd;
        popad;
    }
}
#endif
