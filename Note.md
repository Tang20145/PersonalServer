Note
===========

> 我将我遇到的问题，以及获取的解答（无论是来自ai还是网络或前辈），记录于此  
> 我会用todo格式标明我是否对该项解答进行实践、理解



# 数据库

## 架构

### 观影列表mysql设置问题

问题：

我使用方案:多对多关联表（推荐方案），我的watch list数据表包含下面几个字段：
主键：id int 非空
作品名称：name char[50]（50够不够？），非空
评分：rate uint
状态：status char[10]（用四种字符串表示各种状态，未开始，进行中，完成，放弃
开始观看时间：start_time date
完成观看时间：finish_time date
评论：comment varchar

请你按照方案一，把数据表WatchList，标签表Tags，关联表WatchListPairTags的格式列出，并且提供将带作品类型标签的分页查询语句

结果：

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

问题：

如何部署？

结果：



问题：

docker mysql的时间不对

结果：

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

# 网页

## JavaScript

==Q==

这段js代码我大概看了一些，我初学了js，请你矫正我的理解，\$(document).ready 将 html 的整个文本对象转换为 jquery 对象，然后执行 jquery 对象的 ready 函数，表示当 document 对象准备好后（也就是当客户端完全收到 document 这段 html 的代码之后）开始执行后续的代码，后续代码中 \$('#mediaTable').DataTable 是将 document 中 id 为mediaTable 的元素转为 jquery 对象，并调用其 jquery 的接口 DataTable ，这个看着像一个构造函数，传入的{}内就是一个有点像字面量的对象作为构造依据，但其中的内容我就不懂了，请你详细解释

```javascript
<!--下面这段就是javaScript-->
    <script>
        $(document).ready(function () {
            $('#mediaTable').DataTable({
                serverSide: true,
                ajax: {
                    url: '/api/WatchListFullView', // 服务器接口URL（需替换为你的实际地址）
                    type: 'GET',
                    data: function(d) {
                        // 4. 将DataTables的参数转换为后端需要的格式
                        return {
                        draw: d.draw,         // DataTables安全校验参数（必须回传）
                        page: Math.floor(d.start / d.length) + 1, // 计算页码（从1开始）
                        pageSize: d.length,   // 每页条数
                        keyword: d.search.value || '', // 搜索关键词
                        
                        // 可选：添加排序信息（如后端需要）
                        sortField: d.order[0]?.column,  // 排序列索引
                        sortDirection: d.order[0]?.dir  // asc/desc
                        };
                    }
                },
                "columns": [
                    { "data": "id",
                    "className": "text-nowrap"},
                    { "data": "name" },
                    { "data": "eng_name" },
                    {
                        "data": "tags",
                        "render": function (data) {
                            return data.map(tag =>
                                `<span class="badge bg-primary">${tag}</span>`
                            ).join(' ');
                        },"className": "text-nowrap"
                    },
                    { "data": "type",
                    "className": "text-nowrap" },
                    { "data": "rate",
                    "className": "text-nowrap" },
                    { "data": "status" },
                    { "data": "start_time",
                    "className": "text-nowrap" },
                    { "data": "finish_time",
                    "className": "text-nowrap" },
                    { "data": "comment" },
                    { "data": "link" }
                ]
            });
        });
```

==A==

你的理解基本正确，但可以更精确地拆解和补充细节。以下是逐部分解析：

1. $(document).ready(function () { ... })

• $(document)  

  这是 jQuery 的核心选择器，将原生 DOM 的 document 对象封装成 jQuery 对象，从而可以使用 jQuery 提供的方法（如 .ready()）。
• .ready()  

  是 jQuery 提供的 DOM 就绪事件监听方法，当 HTML 文档的 DOM 结构完全加载并解析完毕（无需等待图片等资源）时触发回调函数。  
  • 底层原理：封装了原生 DOMContentLoaded 事件，提供跨浏览器兼容性。

  • 简写形式：$(function () { ... }) 与 $(document).ready() 完全等价。

2. $('#mediaTable').DataTable({ ... })

• $('#mediaTable')  

  通过 ID 选择器选中 DOM 中 id="mediaTable" 的元素（通常是一个 <table>），并转换为 jQuery 对象。
• .DataTable()  

  是 jQuery DataTables 插件的 初始化方法，用于将普通 HTML 表格转换为功能丰富的交互式表格（支持排序、分页、搜索等）。  
  • 构造函数特性：确实类似构造函数，传入的配置对象 {} 决定了表格的行为和外观。

  • 返回值：返回一个 DataTables API 实例，可通过该实例动态操作表格（如刷新数据、修改配置等）。

3. 配置对象 {} 的详细解析

(1) serverSide: true

• 作用：启用 服务器端模式，所有数据处理（分页、排序、搜索等）由后端完成，前端仅负责展示。  

• 适用场景：数据量较大时，避免一次性加载所有数据到前端。

(2) ajax 配置

• url：指定后端 API 地址，用于获取表格数据。

• data 函数：  

  将 DataTables 自动生成的参数（如分页、搜索条件）转换为后端需要的格式。例如：
  data: function(d) {
    return {
      page: Math.floor(d.start / d.length) + 1, // 当前页码
      pageSize: d.length,                       // 每页条数
      keyword: d.search.value                   // 搜索关键词
    };
  }
  
  • d 是 DataTables 的内部参数对象，包含分页起始索引（start）、每页长度（length）、排序信息（order）等。

(3) columns 配置

• data：指定每列对应的数据字段名（与后端返回的 JSON 数据键名匹配）。

• render：自定义列渲染逻辑。例如将 tags 数组渲染为多个 Bootstrap 徽章：
  render: function (data) {
    return data.map(tag => `<span class="badge bg-primary">${tag}</span>`).join(' ');
  }
  
• className：为列添加 CSS 类（如 text-nowrap 防止文本换行）。

4. DataTables 的工作流程

1. 初始化时：根据配置发起 AJAX 请求，携带分页、排序等参数到后端。
2. 后端处理：按参数查询数据库，返回结构化数据（如 { "data": [...], "total": 100 }）。
3. 前端渲染：DataTables 将数据填充到表格，并生成分页控件、搜索框等交互元素。

5. 常见疑问解答

• 为什么用 serverSide: true？  

  大数据场景下，后端分页和过滤比前端性能更高。
• draw 参数的作用？  

  安全校验参数，确保 AJAX 响应与请求顺序一致（防止竞态条件）。
• 列配置中的 className？  

  直接作用于该列所有单元格，方便统一样式。

总结

你的理解方向完全正确，关键点在于：
1. $(document).ready() 是 jQuery 的 DOM 就绪事件监听。
2. DataTable() 是插件的 初始化方法，配置对象决定了表格的 数据源、交互行为和渲染方式。
3. 服务器端模式（serverSide: true）将计算压力转移给后端，适合大规模数据。