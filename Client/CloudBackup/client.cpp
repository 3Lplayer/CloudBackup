#define _CRT_SECURE_NO_WARNINGS 1
#include <ctime>
#include "file_util.hpp"
#include "data.hpp"
#include "backup.hpp"

#define BACKUP_FILE ".\\backup_file.txt" //�־û��ļ�
#define BACKUP_DIR ".\\BackupDir" //����Ŀ¼

int main()
{
	cloud::Backup(BACKUP_DIR, BACKUP_FILE).Handle();
	return 0;
}