// 固定每页限制，必须与 C++ 后端的 DEFAULT_LIMIT 保持一致
const SQL_LIMIT_DEFAULT = 20; 
// 存储原始行数据，用于取消编辑时恢复
const g_OriginalRowData = new Map();

// 异步函数，用于根据页码获取数据
async function fetchData(page = 1,pageSize = SQL_LIMIT_DEFAULT) {
    console.log('start fetchData');
    // 获取网页元素
    const tableBody = document.getElementById('watch-list-tbody');
    const paginationControls = document.getElementById('watch-list-pagination-controls');
    // 加载前清空现有内容并显示加载提示，横跨8列
    tableBody.innerHTML = '<tr><td colspan="8">正在加载...</td></tr>';
    paginationControls.innerHTML = '';

    // 构造请求 URL，包含 page 和 limit 参数
    const url = `/WatchList?page=${page}&pageSize=${pageSize}`;
    
    console.log('start fetch');
    // 1. 发送请求
    const response = await fetch(url);
    console.log('end fetch');
    if (!response.ok) {
        tableBody.innerHTML = '<tr><td colspan="3">服务器数据获取失败</td></tr>';
        return;
    }
    // 成功获取响应，开始解析 JSON

    jsonResponse = await response.json();
    // jsonResponse 现在就是 C++ 后端返回的 JavaScript 对象
    // 数据表数组
    const data = jsonResponse.data;

    // --- 接下来是“循环读”：渲染表格 ---
    tableBody.innerHTML = ''; // 清空加载提示

    if (data.length === 0) {
        tableBody.innerHTML = '<tr><td colspan="3">没有数据</td></tr>';
    } else 
    {
        // 遍历 data 数组
        data.forEach(item => { 
            // 1. 创建新行 (<tr>)
            const row = tableBody.insertRow(); 
            
            // 2. 插入单元格 (<td>)，这是JavaScript 针对表格设计的 DOM (Document Object Model) 操作方法：
            row.id =`row-${item.id}`;// 这里赋值的是row这个DOM的id属性，不是成员对象
            row.insertCell().textContent = item.id; 
            row.insertCell().textContent = item.name;
            row.insertCell().textContent = item.eng_name;
            row.insertCell().textContent = item.type;
            row.insertCell().textContent = item.tags.join(',');
            row.insertCell().textContent = item.rate;
            row.insertCell().textContent = item.year;
            row.insertCell().textContent = item.start_time;
            row.insertCell().textContent = item.finish_time;
            
        });
    renderPagination(jsonResponse.page,jsonResponse.totalPage);
    }
    console.log('end fetchData');
}



// 页面加载完成后，自动获取第一页数据
document.addEventListener('DOMContentLoaded', () => {
    console.log('start addEventListener');
    fetchData(1);
    console.log('end addEventListener');
});
