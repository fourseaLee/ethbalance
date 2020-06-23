#include "syncer.h"
#include <glog/logging.h>
#include "db_mysql.h"

static void SetTimeout(const std::string& name, int second)
{
    struct timeval timeout ;
    timeout.tv_sec = second;
    timeout.tv_usec = 0;
    evtimer_add(Job::map_name_event_[name], &timeout);
}

static void ScanChain(int fd, short kind, void *ctx)
{
    LOG(INFO) << "scan block begin ";
    Syncer::instance().scanBlockChain(); 
    SetTimeout("ScanChain", 10);
}

void Syncer::refreshDB()
{
	LOG(INFO) << "refresh DB begin" ;
	LOG(INFO) << "SQL size: " << vect_sql_.size() ;
	if (vect_sql_.size() > 0)
	{
		g_db_mysql->batchRefreshDB(vect_sql_);
		vect_sql_.clear();
	}	
	LOG(INFO) << "refresh DB end" ;
}

void Syncer::scanBlockChain()
{
	//check height which is needed to upate
	std::string sql = "select adddress from account ;";
	std::map<int,DBMysql::DataType> map_col_type;
	map_col_type[0] = DBMysql::STRING;

	json json_data;
	g_db_mysql->getData(sql, map_col_type, json_data);
	if (json_data.size() > 0)
	{
		return;
	}

	for(uint i = 0; i < json_data[0].size(); i++)
	{
		std::string address = json_data[0][i].get<std::string>();
		std::string eth = "0";
		std::string usdt = "0";
		rpc_.getBalance(address, eth, usdt);
		//UPDATE `ethdb`.`account` SET `eth`='0', `usdt`='0' WHERE  `address`='0xcd09aa30abe6aec8ebc8dbe7ae8de518d49b1ddf' AND `eth` IS NULL AND `usdt` IS NULL LIMIT 2;
		std::string sql = "UPDATE `account` SET `eth`='" + eth + "', `usdt`='" + usdt + "' WHERE  `address`='" +  address + "';";
		LOG(INFO) << sql;

		vect_sql_.push_back(sql);
	}

	refreshDB();

}


Syncer Syncer::single_;
void Syncer::registerTask(map_event_t& name_events, map_job_t& name_tasks)
{
    REFLEX_TASK(ScanChain);
}


