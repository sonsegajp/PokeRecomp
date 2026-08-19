#include "global.h"
#include "link.h"
#include "platform/net_client.h"
#include "../../server/protocol.h"
#include <stdio.h>
#include <string.h>

extern bool8 gRemoteLinkPlayersNotReceived[];
extern bool8 gReadyToCloseLink[];
extern bool8 gReadyToExitStandby[];

static bool8 sNetLinkActive = FALSE;
#define MAX_PENDING_BLOCKS 16
static u8 sPendingBlocks[MAX_PENDING_BLOCKS][BLOCK_BUFFER_SIZE];
static u16 sPendingBlockSizes[MAX_PENDING_BLOCKS];
static int sPendingBlockCount = 0;
static u8 sNetLinkPartnerId = 0;
static bool8 sNetLinkIsMaster = FALSE;
static u8 sBlockReceivedFlags = 0;
static bool8 sSendComplete = TRUE;
static bool8 sNeedsSelfSend = FALSE;

bool8 NetLink_IsActive(void)
{
    return sNetLinkActive;
}

void NetLink_Init(u8 partnerId, bool8 isMaster)
{
    sNetLinkActive = TRUE;
    sNetLinkPartnerId = partnerId;
    sNetLinkIsMaster = isMaster;
    sBlockReceivedFlags = 0;
    sNeedsSelfSend = FALSE;

    sSendComplete = TRUE;

    gReceivedRemoteLinkPlayers = TRUE;
    gLinkVSyncDisabled = FALSE;
    gLinkStatus = (2 << LINK_STAT_PLAYER_COUNT_SHIFT)
               | (isMaster ? LINK_STAT_MASTER : 0)
               | LINK_STAT_CONN_ESTABLISHED;
    SetSuppressLinkErrorMessage(TRUE);

    for (int j = 0; j < MAX_LINK_PLAYERS; j++)
    {
        gRemoteLinkPlayersNotReceived[j] = FALSE;
        gReadyToCloseLink[j] = FALSE;
        gReadyToExitStandby[j] = FALSE;
    }

    memset(&gLinkPlayers[0], 0, sizeof(struct LinkPlayer));
    memset(&gLinkPlayers[1], 0, sizeof(struct LinkPlayer));

    u8 myIdx = isMaster ? 0 : 1;
    u8 opIdx = isMaster ? 1 : 0;

    u8 mySprite = NetClient_GetMyTrainerSprite();
    u16 myVersion = (mySprite >= 2) ? VERSION_RUBY : VERSION_FIRE_RED;
    u8 myGender = (mySprite == 1 || mySprite == 3) ? 1 : 0;

    gLinkPlayers[myIdx].version = myVersion;
    gLinkPlayers[myIdx].lp_field_2 = 0x8000;
    gLinkPlayers[myIdx].trainerId = gSaveBlock2Ptr->playerTrainerId[0]
        | (gSaveBlock2Ptr->playerTrainerId[1] << 8)
        | (gSaveBlock2Ptr->playerTrainerId[2] << 16)
        | (gSaveBlock2Ptr->playerTrainerId[3] << 24);
    {
        extern const char *NetClient_GetMyUsername(void);
        const char *myName = NetClient_GetMyUsername();
        u8 myNameEnc[PLAYER_NAME_LENGTH + 1];
        int n;
        for (n = 0; n < PLAYER_NAME_LENGTH && myName[n]; n++)
        {
            char c = myName[n];
            if (c >= 'A' && c <= 'Z') myNameEnc[n] = 0xBB + (c - 'A');
            else if (c >= 'a' && c <= 'z') myNameEnc[n] = 0xD5 + (c - 'a');
            else if (c >= '0' && c <= '9') myNameEnc[n] = 0xA1 + (c - '0');
            else myNameEnc[n] = 0x00;
        }
        myNameEnc[n] = 0xFF;
        memcpy(gLinkPlayers[myIdx].name, myNameEnc, PLAYER_NAME_LENGTH + 1);
    }
    gLinkPlayers[myIdx].gender = myGender;
    gLinkPlayers[myIdx].linkType = LINKTYPE_BATTLE;
    gLinkPlayers[myIdx].id = myIdx;
    gLinkPlayers[myIdx].language = LANGUAGE_ENGLISH;

    struct RemotePlayer *remote = &NetClient_GetRemotePlayers()[partnerId];
    u8 opponentSprite = remote->trainerSprite;
    u16 opponentVersion = (opponentSprite >= 2) ? VERSION_RUBY : VERSION_FIRE_RED;
    u8 opponentGender = (opponentSprite == 1 || opponentSprite == 3) ? 1 : 0;

    gLinkPlayers[opIdx].version = opponentVersion;
    gLinkPlayers[opIdx].lp_field_2 = 0x8000;
    gLinkPlayers[opIdx].trainerId = partnerId;
    u8 nameEnc[PLAYER_NAME_LENGTH + 1];
    int i;
    for (i = 0; i < PLAYER_NAME_LENGTH && remote->username[i]; i++)
    {
        char c = remote->username[i];
        if (c >= 'A' && c <= 'Z') nameEnc[i] = 0xBB + (c - 'A');
        else if (c >= 'a' && c <= 'z') nameEnc[i] = 0xD5 + (c - 'a');
        else if (c >= '0' && c <= '9') nameEnc[i] = 0xA1 + (c - '0');
        else nameEnc[i] = 0x00;
    }
    nameEnc[i] = 0xFF;
    memcpy(gLinkPlayers[opIdx].name, nameEnc, PLAYER_NAME_LENGTH + 1);
    gLinkPlayers[opIdx].gender = opponentGender;
    gLinkPlayers[opIdx].linkType = LINKTYPE_BATTLE;
    gLinkPlayers[opIdx].id = opIdx;
    gLinkPlayers[opIdx].language = LANGUAGE_ENGLISH;

    printf("[NET-LINK] Initialized: partner=%d master=%d pending=%d\n", partnerId, isMaster, sPendingBlockCount);
    fflush(stdout);
}

void NetLink_Cleanup(void)
{
    sNetLinkActive = FALSE;
    sBlockReceivedFlags = 0;
    sPendingBlockCount = 0;

    sSendComplete = TRUE;
    gReceivedRemoteLinkPlayers = FALSE;
    gLinkStatus = 0;
    printf("[NET-LINK] Cleaned up\n"); fflush(stdout);
}

bool8 NetLink_SendBlock(const void *src, u16 size)
{
    if (!sNetLinkActive || !NetClient_IsConnected()) return FALSE;
    if (size > LINK_BLOCK_MAX) size = LINK_BLOCK_MAX;

    u8 myId = sNetLinkIsMaster ? 0 : 1;
    memcpy(gBlockRecvBuffer[myId], src, size);
    sBlockReceivedFlags |= (1 << myId);

    extern void NetLink_SendRawBlock(const void *data, u16 size);
    NetLink_SendRawBlock(src, size);
    sSendComplete = TRUE;
    return TRUE;
}

void NetLink_SendRawBlock(const void *data, u16 size)
{
    struct LinkDataPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.header.type = MSG_C_LINK_DATA;
    pkt.header.size = sizeof(struct PacketHeader) + 1 + 2 + size;
    pkt.fromPlayerId = NetClient_GetPlayerId();
    pkt.dataSize = size;
    if (size > LINK_BLOCK_MAX) size = LINK_BLOCK_MAX;
    memcpy(pkt.data, data, size);

    extern int NetClient_SendRaw(const void *data, int size);
    NetClient_SendRaw(&pkt, sizeof(struct PacketHeader) + 1 + 2 + size);
}

void NetLink_OnReceiveBlock(const void *data, u16 size)
{
    if (size > BLOCK_BUFFER_SIZE) size = BLOCK_BUFFER_SIZE;
    if (sPendingBlockCount < MAX_PENDING_BLOCKS)
    {
        memcpy(sPendingBlocks[sPendingBlockCount], data, size);
        sPendingBlockSizes[sPendingBlockCount] = size;
        sPendingBlockCount++;
    }
}

u8 NetLink_GetBlockReceivedStatus(void)
{
    u8 myId = sNetLinkIsMaster ? 0 : 1;
    if (sNeedsSelfSend)
    {
        sNeedsSelfSend = FALSE;
        if (!(sBlockReceivedFlags & (1 << myId)))
        {
            extern u8 gBlockSendBuffer[];
            NetLink_SendBlock(gBlockSendBuffer, BLOCK_BUFFER_SIZE);
        }
    }

    if (sPendingBlockCount > 0)
    {
        u8 partnerId = sNetLinkIsMaster ? 1 : 0;
        if (!(sBlockReceivedFlags & (1 << partnerId)))
        {
            memcpy(gBlockRecvBuffer[partnerId], sPendingBlocks[0], sPendingBlockSizes[0]);
            sBlockReceivedFlags |= (1 << partnerId);
            for (int i = 1; i < sPendingBlockCount; i++)
            {
                memcpy(sPendingBlocks[i - 1], sPendingBlocks[i], sPendingBlockSizes[i]);
                sPendingBlockSizes[i - 1] = sPendingBlockSizes[i];
            }
            sPendingBlockCount--;
        }
    }
    return sBlockReceivedFlags;
}

void NetLink_ResetBlockReceivedFlag(u8 playerId)
{
    sBlockReceivedFlags &= ~(1 << playerId);
}

void NetLink_RequestSelfSend(void)
{
    sNeedsSelfSend = TRUE;
}

bool8 NetLink_IsSendComplete(void)
{
    return sSendComplete;
}

u8 NetLink_GetMultiplayerId(void)
{
    return sNetLinkIsMaster ? 0 : 1;
}
