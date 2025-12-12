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
            // 新增操作列
            const actionCell = row.insertCell();
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

// 切换编辑/只读模式
function toggleEdit(id) {
    const row = document.getElementById(`row-${id}`);
    // 取得除 ID 列和 Actions 列外的所有数据单元格
    // row.cells 是一个类数组对象，所以我们用 Array.from() 转换它
    const editableCells = Array.from(row.cells).slice(1, -1); // 从第二个单元格开始，到倒数第二个结束

    if (row.classList.contains('editing')) {
        // 当前为编辑模式，切换回只读模式
        editableCells.forEach(cell => cell.contentEditable = 'false');
        row.classList.remove('editing');
        // 替换按钮为 Edit
        row.cells[row.cells.length - 1].innerHTML = `<button onclick="toggleEdit(${id})">编辑</button>`;
    } else {
    // 当前为只读模式，切换到编辑模式
    // ** 存储原始数据 **
    const originalData = editableCells.map(cell => cell.textContent);
    originalRowData.set(id, originalData);

    // 设置单元格为可编辑
    editableCells.forEach(cell => cell.contentEditable = 'true');
    row.classList.add('editing'); // 可用于CSS来突出显示正在编辑的行
    // 替换按钮为 Save 和 Cancel
    row.cells[row.cells.length - 1].innerHTML = `
        <button onclick="handleSave(${id})">保存</button>
        <button onclick="handleCancel(${id})">取消</button>
    `;
    }
}
    
// 取消修改
function handleCancel(id) {
    const row = document.getElementById(`row-${id}`);
    const editableCells = Array.from(row.cells).slice(1, -1);
    const originalData = originalRowData.get(id);
    
    if (originalData) {
        // 恢复单元格内容
        editableCells.forEach((cell, index) => {
        cell.textContent = originalData[index];
        });
        originalRowData.delete(id); // 清除存储的原始数据
    }
    
    // 切换回只读模式
    toggleEdit(id);
}
    
// 保存数据到后端
async function handleSave(id) {
    const row = document.getElementById(`row-${id}`);
    const editableCells = Array.from(row.cells).slice(1, -1);

     // 获取新的数据值
    const newData = {
    id: id, // 传回 ID
    name: editableCells[0].textContent,
    eng_name: editableCells[1].textContent,
    tags: editableCells[2].textContent.split(',').map(tag => tag.trim()), // 再次分割 tags
    type: editableCells[3].textContent,
    rate: parseFloat(editableCells[4].textContent), // 转换为数字类型
    start_time: editableCells[5].textContent,
    finish_time: editableCells[6].textContent,
    };

     // 1. 发送 PUT 请求 (更新数据常用 PUT/PATCH)
    const response = await fetch(`/WatchList/${id}`, { // 假设后端使用 RESTful 路径：/WatchList/101
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(newData), // 将 JS 对象转换为 JSON 字符串发送
    });
    
    if (response.ok) {
        console.log(`ID ${id} 数据保存成功`);
        originalRowData.delete(id); // 保存成功，清除原始数据
        toggleEdit(id); // 切换回只读模式
    
        // 最佳实践：保存后可以考虑重新加载当前页的数据，确保数据完全同步
        // 但为节省时间，我们只做模式切换。
    } else {
        alert('保存失败，请检查后端服务或数据格式');
    }
}