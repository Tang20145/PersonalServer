// 固定每页限制，必须与 C++ 后端的 DEFAULT_LIMIT 保持一致
const SQL_LIMIT_DEFAULT = 20; 

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
            row.insertCell().textContent = item.id; 
            row.insertCell().textContent = item.name;
            row.insertCell().textContent = item.eng_name;
            row.insertCell().textContent = item.tags.join(',');
            row.insertCell().textContent = item.type;
            row.insertCell().textContent = item.rate;
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

// 渲染控制按钮，先默认只能控制页面，不控制每页数量
function renderPagination(currentPage, totalPages) {
    console.log('start renderPagination');

    const controls = document.getElementById('watch-list-pagination-controls');
    controls.innerHTML = `总共 ${totalPages} 页 | `; 

    const range = 2; // 只显示当前页前后 2 页
    const startPage = Math.max(1, currentPage - range);
    const endPage = Math.min(totalPages, currentPage + range);
    
    // 循环创建页码链接
    for (let i = startPage; i <= endPage; i++) {
        const span = document.createElement('span');
        span.classList.add('pagination-link'); // 用于 CSS 样式
        span.textContent = `[${i}]`;

        if (i === currentPage) {
            // 当前页码：只改变样式，不可点击
            span.classList.add('current-page');
        } else {
            // 关键步骤：附加 onclick 事件
            // 当点击 span 时，调用 fetchData 函数，传入目标页码 i
            span.onclick = () => fetchData(i);
        }
        
        controls.appendChild(span);
    }
    console.log('end renderPagination');
}