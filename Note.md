Note
===========

> 我将我遇到的问题，以及获取的解答（无论是来自ai还是网络或前辈），记录于此  
> 我会用todo格式标明我是否对该项解答进行实践、理解



# 数据库

## 架构

### 观影列表mysql设置问题

==Q==
我使用方案:多对多关联表（推荐方案），我的watch list数据表包含下面几个字段：
主键：id int 非空
作品名称：name char[50]（50够不够？），非空
评分：rate uint
状态：status char[10]（用四种字符串表示各种状态，未开始，进行中，完成，放弃
开始观看时间：start_time date
完成观看时间：finish_time date
评论：comment varchar

请你按照方案一，把数据表WatchList，标签表Tags，关联表WatchListPairTags的格式列出，并且提供将带作品类型标签的分页查询语句

==A==

- [X]观影列表以及标签表、关联表，思路大致类似下面指令，后续有别的更改不在此更新了
```sql
-- 观影列表
CREATE TABLE `WatchList` (
  `id` int NOT NULL AUTO_INCREMENT COMMENT '主键id',
  `name` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL COMMENT '作品名称',
  `eng_name` varchar(100) CHARACTER SET ascii COLLATE ascii_general_ci DEFAULT NULL COMMENT '作品英文名称',
  `type` enum('剧集','电影','动画','纪录片') CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '电影' COMMENT '作品类型',
  `rate` tinyint unsigned DEFAULT NULL COMMENT '评分(0-10)',
  `status` enum('未开始','进行中','完成','放弃') COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '未开始',
  `start_time` date DEFAULT NULL COMMENT '开始观看日期',
  `finish_time` date DEFAULT NULL COMMENT '完成观看日期',
  `comment` text COLLATE utf8mb4_unicode_ci COMMENT '评论内容',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='方糖的观影列表';

-- 标签表
CREATE TABLE `Tags` (
  `tag_name` varchar(20) NOT NULL COMMENT '标签名作为主键',
  `description` varchar(100) DEFAULT NULL,
  `color` varchar(7) DEFAULT '#6c757d' COMMENT '标签颜色',
  PRIMARY KEY (`tag_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- 关联表，以(`watch_id`,`tag_name`)组合为主键，实现一个作品对应多个标签
CREATE TABLE `WatchList_Tags` (
  `watch_id` int NOT NULL,
  `tag_name` varchar(20) NOT NULL,
  PRIMARY KEY (`watch_id`,`tag_name`),
  KEY `tag_name` (`tag_name`),
  CONSTRAINT `WatchList_Tags_ibfk_1` FOREIGN KEY (`watch_id`) REFERENCES `WatchList` (`id`) ON DELETE CASCADE,
  CONSTRAINT `WatchList_Tags_ibfk_2` FOREIGN KEY (`tag_name`) REFERENCES `Tags` (`tag_name`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- 查询带标签的视图
CREATE OR REPLACE
ALGORITHM = UNDEFINED VIEW `WatchListFullView` AS
select
    `w`.`name` AS `name`,
    `w`.`eng_name` AS `eng_name`,
    group_concat(`t`.`tag_name` separator ',') AS `tags`,
    `w`.`type` AS `type`,
    `w`.`rate` AS `rate`,
    `w`.`status` AS `status`,
    `w`.`start_time` AS `start_time`,
    `w`.`finish_time` AS `finish_time`,
    `w`.`comment` AS `comment`
from
    ((`WatchList` `w`
left join `WatchList_Tags` `wt` on
    ((`w`.`id` = `wt`.`watch_id`)))
left join `Tags` `t` on
    ((`wt`.`tag_name` = `t`.`tag_name`)))
group by
    `w`.`id`;
```

## 部署

==Q==

docker mysql的时间不对

==A==

- [X]我已经运行了容器了，所以使用复制主机的时区文件方案，最好的办法是一开始也写进yml，挂载主机的时区文件

```shell
#1. 从宿主机复制时区文件到 MySQL 容器,mysql-container填你实际的容器名
docker cp /etc/localtime mysql-container:/etc/localtime
docker cp /etc/timezone mysql-container:/etc/timezone

#2. 重启 MySQL（使时区生效）
docker restart mysql-container

#3. 验证
docker exec -it mysql-container date
```

==Q==
运行时报错:
```shell
sudo: unable to resolve host lavm-emt21wmdne
```

==A==
在/etc/hosts文件添加下面内容，帮助识别
```
127.0.0.1       lavm-emt21wmdne
```