//
// Created by bian on 2017/7/6.
//
#include "SAGTD.h"

database * SAGTD(database * originDB, database *db, treeNode* root, int MaxSetNum,float PbThreshold,int maxDepth,
                 int breakFlag, int (*breakFun)(database *, database *, database *, float, int, trackRow *)){
    // 1
    trackSet * AResult = str_main(db,maxBackgroundSetLength);

    // 2 - 8
    int i,j,k,l;
    int flag;   // 守护结点是否在最上层
    database  * matched, * Bj, * Cj, *Ts;
    database * temp, *db_free_temp_1, *db_free_temp_2;
    database  * CjCopy = initDb();      // 用于断点逐步打印
    matched = initDb();
    Bj = initDb();
    Cj = initDb();
    trackRow * background;
    int c_len = 0;    // Cj 的长度

    for(i=0;i<AResult->count;i++){
        background = AResult->trackCollection[i];
        flag = 1;
        c_len=0;

        freeDb(Bj);
        freeDb(Cj);
        freeDb(matched);

        Bj = initDb();
        Cj = initDb();
        matched = matchRowByTrajectory(db,background->tracks,background->count);

        for(j=0;j<row_count;j++){
            flag = 1;               // 查找前置1

            if(matched[j] == NULL)
                continue;
            for(k=0;k<row_count;k++){
                if(matched[k] == NULL)
                    continue;
                // j如果被k的所覆盖
                if(nodeAisCoveredByNodeB(
                        root,
                        getGuardingNodeByRow(originDB,root,matched[j]->id),
                        getGuardingNodeByRow(originDB,root,matched[k]->id)
                )){
                    flag = 0;
                }
            }

            if(flag){
                insertRow(
                        Bj,
                        matched[j]->id,
                        matched[j]->p_level,
                        matched[j]->trajectory,
                        matched[j]->trajectoryCount,
                        matched[j]->disease
                );
            }
        }
//        traverseDb(Bj,printRow);

        for(l=0;l<row_count;l++){
            if(Bj[l] == NULL)
                continue;
            // 如果泄露概率比P大
            if(caculateBreachProbability(
                    originDB,
                    db,
                    root,
                    Bj[l]->id,
                    background->tracks,
                    background->count
            ) > PbThreshold){

                insertRow(Cj,Bj[l]->id,Bj[l]->p_level,Bj[l]->trajectory,Bj[l]->trajectoryCount,Bj[l]->disease);

            }
        }

        // 5-7
        if(getLengthOfDB(Cj) != 0){
            if(breakFlag){
                freeDb(CjCopy);
                CjCopy = initDb();
                CjCopy = DBUnion(CjCopy,Cj);
            }
            // 6
//            traverseDb(Cj,printRow);
            temp = sag(originDB, db,root,background,Cj,maxDepth,PbThreshold);        // Cj 已变
//            traverseDb(temp,printRow);

            db_free_temp_1 = db;
            db = DBSub(db,Cj);

            db_free_temp_2 = db;
            db = DBUnion(db,temp);

            freeDb(db_free_temp_1);
            freeDb(db_free_temp_2);
            freeDb(temp);
            temp = NULL;
        }

        // 根据标识符判断是否打印步骤，用户可以随时退出
        if(breakFlag){
            breakFlag = breakFun(Bj,CjCopy,db,PbThreshold,maxDepth,background);
        }

    }

    Ts = db;
    return Ts;

}