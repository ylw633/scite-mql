SciTE For PHP
打造轻量级的、高效的PHP IDE

重新编译修改项：
1. 增强PHP代码高亮颜色;
2. 增加项目管理文件树, 对项目管理更加方便;
3. 增加二级菜单支持，工具栏、Tab右键菜单、编辑区右键菜单均支持二级菜单;
4. 增强Snippets功能，编写代码效率可以变得更加高效;
5. 修改状态栏,增加部件显示而不用去点击切换，状态栏增加实时显示文件编码;
6. 内置UTF8编码自动判断而非通过lua实现，打开文件速度更快内存占用更低了;
7. 新增OnMenuCommand事件，方便在执行菜单命令行调用lua代码;
8. 新增OnStatusBarClick事件，单击状态栏相应part可以处理不同事件;
9. 新增save.convert.indent属性，可以设置保存时是否统一(已设置的)缩进格式;
10.新增margin.click.bookmark属性，可以配置单击左边空白区域是否增加书签;
11.新增sidebar.dock属性，可以配置是否自动隐藏侧边栏。


基础改进：
1. 右键菜单常用操作项;
3. 文件/项目和函数管理的侧边栏;在侧边栏的文件管理中点右键可以新建文件，如果有选定的了文件，可以新建与之类型相同的文件;
3. 增加颜色选择器,便于调整颜色;
4. php和html页面可以在按F12键在浏览器中预览，如果php文件没有在web服务器根目录下，则会提示拷贝临时文件到根目录下，利用localhost浏览;
5. 自动识别UTF8编码,打开页面不再出现乱码了;
6. 右键增加简单的跳转到当前页class、function、const、variables定义的菜单项;
7. 添加对SVN的支持，不过要求安装TortoiseSVN;
8. 自动PHP Documentor注释功能;
9. 增加html标签匹配lua插件;
10.增加Tab与空格互换lua插件;
11.增加高亮显示选定相同代码插件;
12.其他一些人性化的改进。

==重要说明==

  由于本人能力有限，修改过后的版本可能存在或多或少的问题，希望大家一起来完善！获取源码请发邮件至:ifoosharp@gmail.com。
  本软件包内的很多插件来自第三方，本人只是对其进行了改善，其版权归原作者所有。

==获取最新版本==
Source:  http://code.google.com/p/scite-for-php/
需要源代码直接从svn里checkout!

==字体不清晰的解决办法==
http://hi.baidu.com/it198/blog/item/85edac44b84e1a89b2b7dcd1.html

==SciTE问题/意见/建议反馈==
QQ交流群：16820754 ，Email：ifoosharp@gmail.com

注意: 如果运行SciTE报错, 请将SciTE程序转移至不包含中文的路径下!