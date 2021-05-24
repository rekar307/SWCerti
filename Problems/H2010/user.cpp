//#define HASH unsigned long 
#define NULL 0

//#define DEBUG 
#ifdef DEBUG
//#include <stdio.h>
#else
#define printf //printf
#endif

struct HASH
{
    long long lo;
    long long hi;
};

HASH hash(const char* str)
{
    register HASH tmp;
    tmp.lo = 0;
    tmp.hi = 0;
    register int c;
    register int cnt = 0;

    while (1)
    {
        c = *str++;
        if (c == 0) return tmp;
        tmp.lo = ((tmp.lo << 8) + c);
        cnt++;
        if (cnt == 8) break;
    }
    while (1)
    {
        c = *str++;
        if (c < 'a') break;
        tmp.hi = ((tmp.hi << 8) + c);
    }
    return tmp;
}

const unsigned int MAX_SCHED = 50000;
const unsigned int MAX_USER = 1000;
const unsigned int MAX_GROUP = 100;
const unsigned int MAX_GROUP_MASTER_NUM = 200;
const int kTestGroupId = 46;
const int kTestMasterId = 43;
int schedNo;

struct SCHED
{
    int schedIdx;
    int uid;
    int gid;
    HASH nameH;
    char name[15];
    int child;
    bool bValid;
    SCHED()
    {
        nameH = { -1,-1 };
        for (int i = 0; i < 15; ++i)
            name[i] = '\0';
        schedIdx = uid = gid = child = -1;
        bValid = true;
    }
}sched[MAX_SCHED];

int searchLastChild(int idx) // 현재 idx의 가장 마지막 child indx를 찾는다.
{
    while (idx != -1)
    {
        if (sched[idx].child == -1)
        {
            break;
        }
        idx = sched[idx].child;
    }

    return idx;
}

int createSched(int uid_, int gid_, char name_[])
{
    sched[schedNo].schedIdx = schedNo;
    sched[schedNo].uid = uid_;
    sched[schedNo].gid = gid_;
    sched[schedNo].nameH = hash(name_);
    sched[schedNo].bValid = true;

    return schedNo;
}

struct USER
{
    int schedCnt;
    int schedIdx;
    SCHED* schedList[100];
    USER()
    {
        schedCnt = schedIdx = 0;
        for (register int i = 0; i < 100; ++i)
        {
            schedList[i] = NULL;
        }
    }
}user[MAX_USER];

struct GROUP
{
    int gid;
    int master[MAX_GROUP_MASTER_NUM];
    int masterIdx;
    GROUP()
    {
        gid = -1;
        masterIdx = 0;
        for (register int i = 0; i < MAX_GROUP_MASTER_NUM; i++)
            master[i] = -1;
    }
    // 현재 group의 master 배열에서 -1인 곳을 찾아 업데이트 합니다.
    // 이것은 master 중에 현재 index와 동일한 것이 없다를 의미합니다.
    void insert(int schedIdx)
    {
        for (register int i = 0; i < MAX_GROUP_MASTER_NUM; ++i)
        {
            if (master[i] == -1)
            {
                master[i] = schedIdx;
                masterIdx++;
                return;
            }
        }
#ifdef DEBUG
        printf("[err] fail to insert value: %d into gid: %d\n", schedIdx, gid);
#endif        
    }
}group[MAX_GROUP];

#ifdef DEBUG
void verifyChild(int idx)
{
    if (idx == kTestMasterId || kTestMasterId == -1)
    {
        int curIdx = idx;
        int childIdx = sched[curIdx].child;
        printf("Master: %d -> ", curIdx);
        while (sched[curIdx].child != -1)
        {
            childIdx = sched[curIdx].child;
            printf("Normal: (%s)%d -> ", sched[childIdx].name, childIdx);
            //if (!(sched[curIdx].nameH == sched[childIdx].nameH))
                //printf("Wrong nameH parent: %d, child: %d\n", sched[curIdx].nameH, sched[childIdx].nameH);

            curIdx = sched[curIdx].child;
        }
        printf("NULL\n");
    }
}

void verifyGroup(int gid)
{
    if (gid == kTestGroupId || kTestGroupId == -1)
    {
        printf("Group %d: ", gid);
        for (int i = 0; i < MAX_GROUP_MASTER_NUM; ++i)
        {
            int idx = group[gid].master[i];
            printf("%d-%s(%d) -> ", idx, sched[idx].name, sched[idx].nameH);
            verifyChild(idx);
        }
        printf("NULL\n");
    }
}
#endif

void init()
{
    register USER new_user;
    for (register int i = 0; i < MAX_USER; ++i)
        user[i] = new_user;

    register GROUP new_group;
    for (register int i = 0; i < MAX_GROUP; ++i)
        group[i] = new_group;

    schedNo = 0;
    register SCHED new_sched;
    for (register int i = 0; i < MAX_SCHED; ++i)
        sched[i] = new_sched;

    schedNo = 0;
}

void addEvent(int uid, char ename[], int gid)
{
    // sched 생성
    int new_schedIdx = createSched(uid, gid, ename);
    schedNo++;

    // user에 등록한다.
    user[uid].schedList[user[uid].schedIdx] = &sched[new_schedIdx];
    user[uid].schedIdx++;
    user[uid].schedCnt++;

    // group에 등록한다.
    HASH enameH = hash(ename);
    for (register int i = 0; i < MAX_GROUP_MASTER_NUM; ++i)
    {
        if (group[gid].master[i] != -1) // Master가 있는 경우, hash 확인
        {
            int curIdx = group[gid].master[i];
            if ((sched[curIdx].nameH.lo == enameH.lo) &&  // hash 같으면 child 맨 끝에 달기
                (sched[curIdx].nameH.hi == enameH.hi))
            {
                curIdx = searchLastChild(curIdx);
                sched[curIdx].child = new_schedIdx;
                return;
            }
        }
    }

    // 등록하려는 sched가 master로 들어가야 하는 경우
    group[gid].insert(new_schedIdx);

#ifdef DEBUG
    verifyGroup(gid);
#endif
}

int deleteEvent(int uid, char ename[])
{
    register HASH delNameH = hash(ename);
    int gid = -1;
    int curIdx = -1;
    int count = 0;
    int userSchedIdx = -1;

    for (register int i = 0; i < user[uid].schedIdx; ++i)
    {
        if (user[uid].schedList[i]->bValid == true)
        {
            curIdx = user[uid].schedList[i]->schedIdx;
            if ((sched[curIdx].nameH.lo == delNameH.lo) && (sched[curIdx].nameH.hi == delNameH.hi))
            {
                userSchedIdx = i;
                gid = user[uid].schedList[i]->gid;
                break;
            }
        }
    }

    for (register int i = 0; i < MAX_GROUP_MASTER_NUM; ++i)
    {
        if (group[gid].master[i] == -1)
            continue;

        int curIdx = group[gid].master[i];
        if ((sched[curIdx].nameH.lo == delNameH.lo) && (sched[curIdx].nameH.hi == delNameH.hi))
        {
            int masterIdx = curIdx;
            if (sched[curIdx].uid == uid) // Master와 uid까지 같으면 Master이하 바꿈.
            {
                // 1. Master 무효화
                group[gid].master[i] = -1;

                // 2. 전체 child 삭제
                while (curIdx != -1)
                {
                    sched[curIdx].bValid = false;
                    user[sched[curIdx].uid].schedCnt--;

                    curIdx = sched[curIdx].child;
                    count++;
                }

                return count;
            }
            else // Normal -> 이하를 찾아서 삭제
            {
                while (curIdx != -1)
                {
                    int curChildIdx = sched[curIdx].child;
                    if (curChildIdx == -1)
                    {
#ifdef DEBUG
                        printf("[WARN] curChildIdx : -1\n");
#endif
                    }

                    if (sched[curChildIdx].uid == uid)
                    {
                        // child 연결
                        sched[curIdx].child = sched[curChildIdx].child;

                        // change sched update
                        sched[curChildIdx].bValid = false;
                        user[uid].schedCnt--;

#ifdef DEBUG
                        verifyGroup(gid);
#endif
                        return 1;
                    }

                    curIdx = sched[curIdx].child;
                }
            }
        }
    }

    return count;
}

int changeEvent(int uid, char ename[], char cname[])
{
    register HASH chgNameH = hash(ename);
    register HASH newNameH = hash(cname);
    int gid = -1;
    int curIdx = -1;
    int count = 0;
    int userSchedIdx = -1;

    for (register int i = 0; i < user[uid].schedIdx; ++i)
    {
        if (user[uid].schedList[i]->bValid == true)
        {
            curIdx = user[uid].schedList[i]->schedIdx;
            if ((sched[curIdx].nameH.lo == chgNameH.lo) && (sched[curIdx].nameH.hi == chgNameH.hi))
            {
                userSchedIdx = i;
                gid = user[uid].schedList[i]->gid;
                break;
            }
        }
    }

    for (register int i = 0; i < MAX_GROUP_MASTER_NUM; ++i)
    {
        if (group[gid].master[i] == -1)
            continue;

        int curIdx = group[gid].master[i];

        if ((sched[curIdx].nameH.lo == chgNameH.lo) && (sched[curIdx].nameH.hi == chgNameH.hi))
        {
            int masterIdx = curIdx;
            if (sched[curIdx].uid == uid) // Master 라면..전체 child 업데이트
            {
                while (curIdx != -1)
                {
                    sched[curIdx].nameH = newNameH;
                    curIdx = sched[curIdx].child;
                    count++;
                }

                return count;
            }
            else // Normal이라면 이하를 찾아서 update
            {
                while (curIdx != -1)
                {
                    int curChildIdx = sched[curIdx].child;

                    if (sched[curChildIdx].uid == uid) // 찾았을때, Normal -> Master로 뺀다
                    {
                        // child 연결
                        sched[curIdx].child = sched[curChildIdx].child;

                        // change sched update
                        sched[curChildIdx].nameH = newNameH;

                        // Master 의 맨 끝에 연결
                        group[gid].insert(curChildIdx);
                        sched[curChildIdx].child = -1;

                        return 1;
                    }
                    curIdx = sched[curIdx].child;
                }
            }
        }
    }

    return count;
}

int getCount(int uid)
{
    return user[uid].schedCnt;
}