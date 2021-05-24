#define MAX_SCHEDULE    50000
#define MAX_USER        1000
#define MAX_GROUP       100
#define MAX_USCHEDULE   100
#define HASH            403
#define NULLPTR         0

void mstrcpy(char dst[], const char src[]) {
    int c = 0;
    while ((dst[c] = src[c]) != 0) ++c;
}

int mstrcmp(const char str1[], const char str2[]) {
    int c = 0;
    while (str1[c] != 0 && str1[c] == str2[c]) ++c;
    return str1[c] - str2[c];
}

struct SCHEDULE {
    long long name;
    int firstname;
    int gid;
    int uid;
    bool isMaster;
    bool isDeleted;
    SCHEDULE* next;
};
SCHEDULE schedules[MAX_SCHEDULE + 2];
int sidx;

struct GROUP {
    SCHEDULE hashtb[HASH];
};
GROUP groups[MAX_GROUP];

struct USER {
    int cnt;
    int schdule[MAX_USCHEDULE];
};
USER users[MAX_USER];

void init()
{
    sidx = 0;
    for (register int i = 0; i < MAX_USER; i++) {
        users[i].cnt = 0;
    }
    for (register int i = 0; i < MAX_GROUP; i++) {
        for (register int j = 0; j < HASH; j++) {
            groups[i].hashtb[j].name = 0;
            groups[i].hashtb[j].firstname = 0;
            groups[i].hashtb[j].next = NULLPTR;
        }
    }
}

int firstName;
long long getName(char str[]) {
    register long long res = 0;
    firstName = ((str[0] - 'a' + 1) << 5) + str[1] - 'a' + 1;
    for (register int i = 2; str[i]; i++) {
        res = (res << 5) + str[i] - 'a' + 1;
    }
    return res;
}

void addEvent(int uid, char ename[], int groupid)
{
    users[uid].schdule[users[uid].cnt++] = sidx;

    register SCHEDULE* sch = &schedules[sidx++];
    sch->name = getName(ename);
    sch->firstname = firstName;
    sch->gid = groupid;
    sch->uid = uid;
    sch->isDeleted = false;
    for (register int idx = sch->name % HASH; ; idx = (idx + 1) % HASH) {

        if ((groups[groupid].hashtb[idx].next == NULLPTR && groups[groupid].hashtb[idx].name == 0) || // 동일한 이름의 일정이 없음
            (groups[groupid].hashtb[idx].name == sch->name && groups[groupid].hashtb[idx].firstname == sch->firstname)) { // 동일한 이름의 일정이 있음

            if (groups[groupid].hashtb[idx].next == NULLPTR) sch->isMaster = true; // 동일한 이름의 일정 없는 경우 마스터
            else sch->isMaster = false;
            sch->next = groups[groupid].hashtb[idx].next;
            groups[groupid].hashtb[idx].next = sch;
            groups[groupid].hashtb[idx].name = sch->name;
            groups[groupid].hashtb[idx].firstname = sch->firstname;
            break;
        }
    }
}

int deleteEvent(int uid, char ename[])
{
    register SCHEDULE* sch;
    register long long name = getName(ename);
    register int idx, cnt;

    for (register int i = 0; i < users[uid].cnt; i++) {
        idx = users[uid].schdule[i];

        if (schedules[idx].name != name || schedules[idx].firstname != firstName) continue;

        sch = &schedules[idx];
        if (!schedules[idx].isMaster) { // 마스터 일정이 아닌 경우
            users[uid].schdule[i] = users[uid].schdule[users[uid].cnt - 1];
            users[uid].cnt--;
            schedules[idx].isDeleted = true;
            return 1;
        }
        // 마스터 일정인 경우
        register int gid = schedules[idx].gid;
        for (register int idx = sch->name % HASH;; idx = (idx + 1) % HASH) {
            if ((groups[gid].hashtb[idx].name == sch->name && groups[gid].hashtb[idx].firstname == sch->firstname)) {
                cnt = 0;
                for (sch = groups[gid].hashtb[idx].next; sch; sch = sch->next) {
                    if (sch->isDeleted) continue;
                    for (register int j = 0; j < users[sch->uid].cnt; j++) {
                        if (schedules[users[sch->uid].schdule[j]].name != sch->name || schedules[users[sch->uid].schdule[j]].firstname != sch->firstname) continue;
                        users[sch->uid].schdule[j] = users[sch->uid].schdule[users[sch->uid].cnt - 1];
                        users[sch->uid].cnt--;
                        break;
                    }
                    sch->isDeleted = true;
                    cnt++;
                }
                groups[gid].hashtb[idx].next = NULLPTR;
                return cnt;
            }
        }
    }
    return 0;
}

int changeEvent(int uid, char ename[], char cname[])
{
    register SCHEDULE* sch1;
    register SCHEDULE* sch2;
    register int firstname1;
    register int idx;
    register int cnt = 0;
    register long long name = getName(ename);
    firstname1 = firstName;

    sch2 = &schedules[sidx++];
    sch2->name = getName(cname);
    sch2->firstname = firstName;
    sch2->uid = uid;
    sch2->isDeleted = false;
    sch2->isMaster = true;

    for (register int i = 0; i < users[uid].cnt; i++) {
        idx = users[uid].schdule[i];
        if (schedules[idx].name != name || schedules[idx].firstname != firstname1) continue;
        sch1 = &schedules[idx];

        users[uid].schdule[i] = sidx - 1;
        sch2->gid = schedules[idx].gid;

        register int changeHashIndex;
        for (register int index = sch2->name % HASH; ; index = (index + 1) % HASH) {
            if (groups[sch2->gid].hashtb[index].next == NULLPTR && groups[sch2->gid].hashtb[index].name == 0) {
                changeHashIndex = index;
                groups[sch2->gid].hashtb[index].name = sch2->name;
                groups[sch2->gid].hashtb[index].firstname = sch2->firstname;
                break;
            }
        }

        if (!schedules[idx].isMaster) { // 마스터가 아닌 경우
            schedules[idx].isDeleted = true;
            sch2->next = groups[sch2->gid].hashtb[changeHashIndex].next;
            groups[sch2->gid].hashtb[changeHashIndex].next = sch2;
            return 1;
        }

        register int groupid = schedules[idx].gid;
        for (register int index = sch1->name % HASH; ; index = (index + 1) % HASH) {
            if ((groups[groupid].hashtb[index].name == sch1->name && groups[groupid].hashtb[index].firstname == sch1->firstname)) {
                cnt = 0;
                register SCHEDULE* sch = groups[groupid].hashtb[index].next;
                for (; sch; sch = sch->next) {
                    if (sch->isDeleted)continue;
                    sch->name = sch2->name;
                    sch->firstname = sch2->firstname;

                    cnt++;
                }
                sch = groups[groupid].hashtb[index].next;
                groups[groupid].hashtb[changeHashIndex].next = sch;
                groups[groupid].hashtb[index].next = NULLPTR;
                break;
            }
        }

    }
    return cnt;
}

int getCount(int uid)
{
    return users[uid].cnt;
}