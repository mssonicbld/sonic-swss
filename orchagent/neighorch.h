#ifndef SWSS_NEIGHORCH_H
#define SWSS_NEIGHORCH_H

#include "orch.h"
#include "observer.h"
#include "portsorch.h"
#include "intfsorch.h"
#include "fdborch.h"

#include "ipaddress.h"
#include "nexthopkey.h"
#include "producerstatetable.h"
#include "schema.h"

#define NHFLAGS_IFDOWN                  0x1 // nexthop's outbound i/f is down

typedef NextHopKey NeighborEntry;

struct NextHopEntry
{
    sai_object_id_t     next_hop_id;    // next hop id
    int                 ref_count;      // reference count
    uint32_t            nh_flags;       // flags
};

struct NeighborData
{
    MacAddress    mac;
    bool          hw_configured = false; // False means, entry is not written to HW
};

/* NeighborTable: NeighborEntry, neighbor MAC address */
typedef map<NeighborEntry, NeighborData> NeighborTable;
/* NextHopTable: NextHopKey, NextHopEntry */
typedef map<NextHopKey, NextHopEntry> NextHopTable;

struct NeighborUpdate
{
    NeighborEntry entry;
    MacAddress mac;
    bool add;
};

class NeighOrch : public Orch, public Subject, public Observer
{
public:
    NeighOrch(DBConnector *db, string tableName, IntfsOrch *intfsOrch, FdbOrch *fdbOrch, PortsOrch *portsOrch);
    ~NeighOrch();

    bool hasNextHop(const NextHopKey&);

    sai_object_id_t getNextHopId(const NextHopKey&);
    sai_object_id_t getLocalNextHopId(const NextHopKey&);
    int getNextHopRefCount(const NextHopKey&);

    void increaseNextHopRefCount(const NextHopKey&, uint32_t count = 1);
    void decreaseNextHopRefCount(const NextHopKey&, uint32_t count = 1);

    bool getNeighborEntry(const NextHopKey&, NeighborEntry&, MacAddress&);
    bool getNeighborEntry(const IpAddress&, NeighborEntry&, MacAddress&);

    bool enableNeighbor(const NeighborEntry&);
    bool disableNeighbor(const NeighborEntry&);
    bool isHwConfigured(const NeighborEntry&);

    sai_object_id_t addTunnelNextHop(const NextHopKey&);
    bool removeTunnelNextHop(const NextHopKey&);

    bool ifChangeInformNextHop(const string &, bool);
    bool isNextHopFlagSet(const NextHopKey &, const uint32_t);
    bool removeOverlayNextHop(const NextHopKey &);
    void update(SubjectType, void *);

    void resolveNeighbor(const NeighborEntry &);

private:
    PortsOrch *m_portsOrch;
    IntfsOrch *m_intfsOrch;
    FdbOrch *m_fdbOrch;
    ProducerStateTable m_appNeighResolveProducer;

    NeighborTable m_syncdNeighbors;
    NextHopTable m_syncdNextHops;

    std::set<NextHopKey> m_neighborToResolve;

    bool addNextHop(const IpAddress&, const string&);
    bool removeNextHop(const IpAddress&, const string&);

    bool addNeighbor(const NeighborEntry&, const MacAddress&);
    bool removeNeighbor(const NeighborEntry&, bool disable = false);

    bool setNextHopFlag(const NextHopKey &, const uint32_t);
    bool clearNextHopFlag(const NextHopKey &, const uint32_t);

    void processFDBFlushUpdate(const FdbFlushUpdate &);

    void doTask(Consumer &consumer);

    bool resolveNeighborEntry(const NeighborEntry &, const MacAddress &);
    void clearResolvedNeighborEntry(const NeighborEntry &);
};

#endif /* SWSS_NEIGHORCH_H */
