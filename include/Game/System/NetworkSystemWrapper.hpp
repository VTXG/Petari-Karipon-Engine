#pragma once

#include <private/iostypes.h>
#include <revolution/types.h>

struct MACAddress {
    /* 0x00 */ u8 mOctets[6];
};

struct IPAddress {
    union {
        /* 0x00 */ u32 mAddress;
        /* 0x00 */ u8 mOctets[4];
    };
};

struct SockAddress {
    /* 0x00 */ u8 mLength;
    /* 0x01 */ u8 mFamily;
    /* 0x02 */ u16 mPort;
    /* 0x04 */ IPAddress mIP;
};

enum SOFamily {
    SO_AF_INET = 2,
};

enum SOType {
    SO_SOCK_STREAM = 1,
    SO_SOCK_DGRAM = 2,
};

enum SOMessageFlags {
    SO_MSG_NONE = 0x00,
    SO_MSG_OOB = 0x01,
    SO_MSG_PEEK = 0x02,
    SO_MSG_NONBLOCK = 0x04,
};

enum SOOptionsFlags {
    SO_OPT_REUSEADDR = 0x4,
    SO_OPT_LINGER = 0x80,
    SO_OPT_OOBINLINE = 0x100,
    SO_OPT_SNDBUF = 0x1001,
    SO_OPT_RCVBUF = 0x1002,
    SO_OPT_SNDLOWAT = 0x1003,
    SO_OPT_RCVLOWAT = 0x1004,
    SO_OPT_TYPE = 0x1008,
    SO_OPT_ERROR = 0x1009,
};

enum SOReturnCode {
    SO_SUCCESS = 0,
    SO_ERR_2BIG = -1,
    SO_ERR_ACCES = -2,
    SO_ERR_ADDRINUSE = -3,
    SO_ERR_ADDRNOTAVAIL = -4,
    SO_ERR_AFNOSUPPORT = -5,
    SO_ERR_AGAIN = -6,
    SO_ERR_ALREADY = -7,
    SO_ERR_BADF = -8,
    SO_ERR_BADMSG = -9,
    SO_ERR_BUSY = -10,
    SO_ERR_CANCELED = -11,
    SO_ERR_CHILD = -12,
    SO_ERR_CONNABORTED = -13,
    SO_ERR_CONNREFUSED = -14,
    SO_ERR_CONNRESET = -15,
    SO_ERR_DEADLK = -16,
    SO_ERR_DESTADDRREQ = -17,
    SO_ERR_DOM = -18,
    SO_ERR_DQUOT = -19,
    SO_ERR_EXIST = -20,
    SO_ERR_FAULT = -21,
    SO_ERR_FBIG = -22,
    SO_ERR_HOSTUNREACH = -23,
    SO_ERR_IDRM = -24,
    SO_ERR_ILSEQ = -25,
    SO_ERR_INPROGRESS = -26,
    SO_ERR_INTR = -27,
    SO_ERR_INVAL = -28,
    SO_ERR_IO = -29,
    SO_ERR_ISCONN = -30,
    SO_ERR_ISDIR = -31,
    SO_ERR_LOOP = -32,
    SO_ERR_MFILE = -33,
    SO_ERR_MLINK = -34,
    SO_ERR_MSGSIZE = -35,
    SO_ERR_MULTIHOP = -36,
    SO_ERR_NAMETOOLONG = -37,
    SO_ERR_NETDOWN = -38,
    SO_ERR_NETRESET = -39,
    SO_ERR_NETUNREACH = -40,
    SO_ERR_NFILE = -41,
    SO_ERR_NOBUFS = -42,
    SO_ERR_NODATA = -43,
    SO_ERR_NODEV = -44,
    SO_ERR_NOENT = -45,
    SO_ERR_NOEXEC = -46,
    SO_ERR_NOLCK = -47,
    SO_ERR_NOLINK = -48,
    SO_ERR_NOMEM = -49,
    SO_ERR_NOMSG = -50,
    SO_ERR_NOPROTOOPT = -51,
    SO_ERR_NOSPC = -52,
    SO_ERR_NOSR = -53,
    SO_ERR_NOSTR = -54,
    SO_ERR_NOSYS = -55,
    SO_ERR_NOTCONN = -56,
    SO_ERR_NOTDIR = -57,
    SO_ERR_NOTEMPTY = -58,
    SO_ERR_NOTSOCK = -59,
    SO_ERR_NOTSUP = -60,
    SO_ERR_NOTTY = -61,
    SO_ERR_NXIO = -62,
    SO_ERR_OPNOTSUPP = -63,
    SO_ERR_OVERFLOW = -64,
    SO_ERR_PERM = -65,
    SO_ERR_PIPE = -66,
    SO_ERR_PROTO = -67,
    SO_ERR_PROTONOSUPPORT = -68,
    SO_ERR_PROTOTYPE = -69,
    SO_ERR_RANGE = -70,
    SO_ERR_ROFS = -71,
    SO_ERR_SPIPE = -72,
    SO_ERR_SRCH = -73,
    SO_ERR_STALE = -74,
    SO_ERR_TIME = -75,
    SO_ERR_TIMEDOUT = -76,
    SO_ERR_TXTBSY = -77,
    SO_ERR_XDEV = -78,
};

class NetworkSystemWrapper {
public:
    enum State {
        STATE_INACTIVE,
        STATE_BUSY,
        STATE_ACTIVE,
    };

    /// @brief Creates a new `NetworkSystemWrapper`.
    NetworkSystemWrapper();

    /// @brief Destroys the `NetworkSystemWrapper`.
    ~NetworkSystemWrapper();

    /// @brief Initializes the network system asynchronously.
    /// @param wait If this function should wait for the initialization process to end.
    void initSystem(bool wait = false);

    /// @brief Closes the network system.
    void closeSystem();

    /// @brief Creates a socket.
    /// @param domain Socket family.
    /// @param type Socket type.
    /// @param protocol Socket protocol.
    /// @return The new socket file descriptor or an error code.
    IOSFd socket(SOFamily domain, SOType type, u32 protocol);

    /// @brief Closes a socket.
    /// @param fd Socket file descriptor.
    /// @return An error code.
    IOSError close(IOSFd fd);

    /// @brief Binds a socket to a specific address.
    /// @param fd Socket file descriptor.
    /// @param rAddress Address to bind to.
    /// @return An error code.
    IOSError bind(IOSFd fd, const SockAddress& rAddress);

    /// @brief Connects a socket to a specific address.
    /// @param fd Socket file descriptor.
    /// @param rAddress Address to connect to.
    /// @note This function should only be used with TCP sockets.
    /// @return An error code.
    IOSError connect(IOSFd fd, const SockAddress& rAddress);

    /// @brief Receives data through a socket.
    /// @param fd Socket file descriptor.
    /// @param pBuffer Pointer to the buffer to copy the data to.
    /// @param size Size of the buffer to copy the data to.
    /// @param flags Operation flags.
    /// @param pAddress Optional sender socket address.
    /// @return An error code.
    IOSError recv(IOSFd fd, void* pBuffer, u32 size, SOMessageFlags flags, SockAddress* pAddress = nullptr);

    /// @brief Sends data through a socket.
    /// @param fd Socket file descriptor.
    /// @param pBuffer Pointer to the data to send.
    /// @param size Size of the data to send.
    /// @param flags Operation flags.
    /// @param rAddress Reference to the address to send the data to.
    /// @return An error code.
    IOSError send(IOSFd fd, void* pBuffer, u32 size, SOMessageFlags flags, const SockAddress& rAddress);

    /// @brief Checks if the network system state is active.
    /// @returns True if the network system state is active; Otherwise false.
    bool isActive() const {
        return mState == STATE_ACTIVE;
    }

    /// @brief Gets the local IP address.
    /// @returns Reference to the local IP address.
    const IPAddress& getLocalIP() const {
        return mIP;
    }

    /// @brief Gets a `NetworkSystemWrapper` instance.
    /// @returns Pointer to the `NetworkSystemWrapper` instance.
    static NetworkSystemWrapper* get();

private:
    void callbackInit();
    void resetInternalState(State state);

    /* 0x00 */ IOSFd mFd;
    /* 0x04 */ IOSError mError;
    /* 0x08 */ State mState;
    /* 0x0C */ IPAddress mIP;
};
