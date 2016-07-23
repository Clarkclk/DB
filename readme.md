
数据库大作业
====

检查点2 系统管理模块
---
* 开发环境：Windows 7(x64)，Qt5(console application)
* 编译选项：必须支持C++11开关，如g++ -std=c++11
* 实现的功能：
        * 借助LEX/YACC工具（GnuWin32 flex和GnuWin32 bison）实现解析器来解析用户的命令并执行相关操作，包括创建、删除、切换和列举数据库信息，创建、删除和列举表信息。
        * 上一检查点已经完成的任务
                * 新建数据库（文件）、删除数据库（删除文件，实现在`FileManager`中）、打开数据库（文件）、关闭数据库（文件）
                * 新建表（设置表的列格式）、插入记录、删除记录、更新记录、获取属性值满足特定条件的记录
* 简要说明：
        * `yacc_lex`目录下的`SQL_Handler.l`主要定义了该解释器的文法，主要包括关键字、数字和标识符的识别。另外，在该文件中，我们将main函数放置在此，实现了Terminal界面接口，并且适当调用`yy_scan_string`, `yyparse`和`yy_delete_buffer`等函数，配合`DBMS`类完成相应任务。
        * `yacc_lex`目录下的`SQL_Handler.y`主要定义了该解释器的语法，主要包括所有本次实现的查询和修改语句。其中较为复杂的是`CREATE TABLE`中的`AttrList`。YACC工具能够很方便地定义语法并且建立语法树，根据这样的语法树调用相应的函数即可完成这个阶段的任务。

                根据C++语言的特点，我们将YACC语法中左值的类型设置为`(void *)`，并且手动维护指针的分配和回收，为此定义了宏`REF(type, pointer)`和`DEL(type, pointer)`。
        * `DBMS`类是Terminal和数据库对象之间交互的接口，它接收来自Terminal的指令，调用数据库对象的方法来完成任务，并且提供有用的信息。
        * 对上一检查点代码的更新
                * 修正了`Database`类中一处bug：保存数据库时遗漏了重要信息（`int curPage`）。
                * 更改了`Database`类中分配页的方案：使用bitMap维护已分配的页；支持回收页（当`Table`删除时）。
                * 对`Database`类增加了`dropTable`函数；修正了`Database`类打开文件失败的返回值；更改了`Database`类构造和析构函数的逻辑，使其自动管理相应的FileManager和BufPageManager。
                * 修改了`FileManager`类打开文件失败的返回值。
                * 为数据表的列信息增加了`specialType`，用于指示该列是否具有`NOT NULL`或`PRIMARY KEY`属性。
        * 上一检查点已经完成的任务
                * `Database`类负责数据库对象和相应的文件管理。按照建议使用文件、数据库一对一的关系进行维护，并且采用`BufPageManager`提高访问效率。
                * `Database`中主要存储有`Table`列表、分配的页信息等。
                * `Table`类负责表对象、记录和相应的文件管理。`Table`中主要存储有列信息、记录数据页信息等。
                * `Table`类中实现了插入和查询操作，`Record`辅助类实现了更新和删除。`Table`类还提供对记录的基本管理逻辑，如按需添加新的数据页等。
                * 主要支持的较为复杂的功能有：维护数据页空闲区域、对记录进行更改和条件查询等
                * `Record`类(table.h/cpp)主要封装记录。其列信息参考所依赖的表，故构造时需要表提供相应信息以便做出更新记录等操作。
                * `RecordCondition`类(table.h/cpp)是为查询条件设计的条件类。
                * `DataType`类是数据库数据类型，目前包括`INT`、`FLOAT`和`VARCHAR`。支持任意定长`VARCHAR`（但单个记录总长应小于8K左右）。
* 测试样例（需将testDB.db文件放置在可执行程序相同目录下）：

                Database terminal.
                DB course project. Jinxu Zhao, Likang Chen. v2.0

                > SHOW DATABASE
                *** Error: syntax error
                testDB> SHOW DATABASE testDB
                Database 'testDB':
                         info   @page:  1
                           tb   @page:  2
                           ii   @page:  3
                testDB> SHOW TABLE info
                Table 'info': (
                        id INT,
                        name VARCHAR(255),
                )
                #record = 0
                testDB> SHOW TABLE tb
                Table 'tb': (
                        id INT NOT NULL, PRIMARY KEY(id),
                )
                #record = 0
                testDB> SHOW TABLE ii
                Table 'ii': (
                        id INT NOT NULL, PRIMARY KEY(id),
                        name VARCHAR(255),
                )
                #record = 0
                testDB> SHOW TABLE i
                ** Error: Table 'i' not found.
                testDB> CREATE DATABASE db
                * Success: Database 'db' created!
                > USE DATABASE db
                * Success: Database 'db' loaded (size = 1 page(s)).
                db> SHOW DATABASE db
                Database 'db':
                db> CREATE TABLE mytable (a INT NOT NULL, b VARCHAR(25) NOT NULL, c INT, PRIMARY KE
                Y(c))
                * Success: Table 'mytable' created.
                db> SHOW TABLE mytable
                Table 'mytable': (
                        a INT NOT NULL,
                        b VARCHAR(25) NOT NULL,
                        c INT, PRIMARY KEY(c),
                )
                #record = 0
                db> USE DATABASE testDB
                * Success: Database 'testDB' loaded (size = 4 page(s)).
                testDB> USE DATABASE db
                * Success: Database 'db' loaded (size = 2 page(s)).
                db> SHOW DATABASE db
                Database 'db':
                        mytable @page:  1
                db> SHOW TABLE mytable
                Table 'mytable': (
                        a INT NOT NULL,
                        b VARCHAR(25) NOT NULL,
                        c INT, PRIMARY KEY(c),
                )
                #record = 0
                db> exit
                Database manage system quits normally.

检查点3 查询解析模块
---

* 开发环境：Windows 7(x64)，Qt5(console application)
* 编译选项：必须支持C++11开关，如g++ -std=c++11
* 实现的功能：
	* 解析和实现了插入、删除、更新和选择语句
	* 支持三个表以上的连接
	* 上一检查点已经完成的任务
		* 借助LEX/YACC工具（GnuWin32 flex和GnuWin32 bison）实现解析器来解析用户的命令并执行相关操作，包括创建、删除、切换和列举数据库信息，创建、删除和列举表信息。
		* 新建数据库（文件）、删除数据库（删除文件，实现在`FileManager`中）、打开数据库（文件）、关闭数据库（文件）
		* 新建表（设置表的列格式）、插入记录、删除记录、更新记录、获取属性值满足特定条件的记录
* 简要说明：
	* 支持所有文档中说明的功能，对于给定的SQL语句都可以正确执行。
	* 对上一检查点代码的更新
		* 对`specialType`的`NOT NULL`属性支持更为完善了，对于插入、更新和查询操作均支持NULL值的查找和判断。
	* 上一检查点已经完成的任务
		* `yacc_lex`目录下的`SQL_Handler.l`主要定义了该解释器的文法，主要包括关键字、数字和标识符的识别。另外，在该文件中，我们将main函数放置在此，实现了Terminal界面接口，并且适当调用`yy_scan_string`, `yyparse`和`yy_delete_buffer`等函数，配合`DBMS`类完成相应任务。
		* `yacc_lex`目录下的`SQL_Handler.y`主要定义了该解释器的语法，主要包括所有本次实现的查询和修改语句。其中较为复杂的是`CREATE TABLE`中的`AttrList`。YACC工具能够很方便地定义语法并且建立语法树，根据这样的语法树调用相应的函数即可完成这个阶段的任务。
		
			根据C++语言的特点，我们将YACC语法中左值的类型设置为`(void *)`，并且手动维护指针的分配和回收，为此定义了宏`REF(type, pointer)`和`DEL(type, pointer)`。
		* `DBMS`类是Terminal和数据库对象之间交互的接口，它接收来自Terminal的指令，调用数据库对象的方法来完成任务，并且提供有用的信息。
		* `Database`类负责数据库对象和相应的文件管理。按照建议使用文件、数据库一对一的关系进行维护，并且采用`BufPageManager`提高访问效率。
		* `Database`中主要存储有`Table`列表、分配的页信息等。
		* `Table`类负责表对象、记录和相应的文件管理。`Table`中主要存储有列信息、记录数据页信息等。
		* `Table`类中实现了插入和查询操作，`Record`辅助类实现了更新和删除。`Table`类还提供对记录的基本管理逻辑，如按需添加新的数据页等。
		* 主要支持的较为复杂的功能有：维护数据页空闲区域、对记录进行更改和条件查询等
		* `Record`类(table.h/cpp)主要封装记录。其列信息参考所依赖的表，故构造时需要表提供相应信息以便做出更新记录等操作。
		* `RecordCondition`类(table.h/cpp)是为查询条件设计的条件类。
		* `DataType`类是数据库数据类型，目前包括`INT`、`FLOAT`和`VARCHAR`。支持任意定长`VARCHAR`（但单个记录总长应小于8K左右）。
* 测试样例（需将testDB.db文件放置在可执行程序相同目录下）：

		Database terminal.
		DB course project. Jinxu Zhao, Likang Chen. v2.0

		> use testDB
		* Success: Database 'testDB' loaded (size = 7 page(s)).
		testDB> show database testDB
		Database 'testDB':
		         user   @page:  1
		        course  @page:  3
		        user_course     @page:  5
		testDB> show user
		Table 'user': (
		        id INT NOT NULL,
		        name VARCHAR(255),
		)
		#record = 5
		------------
		(1, 'Jimmy')
		(2, 'Gladys')
		(3, 'xx')
		(4, 'yy')
		(5, NULL)
		testDB> show course
		Table 'course': (
		        id INT NOT NULL,
		        teacher_id INT NOT NULL,
		        name VARCHAR(255),
		)
		#record = 2
		------------
		(1, 3, 'Math')
		(2, 4, 'English')
		testDB> show user_course
		Table 'user_course': (
		        user INT NOT NULL,
		        course INT NOT NULL,
		)
		#record = 3
		------------
		(1, 1)
		(1, 2)
		(2, 1)
		testDB> SELECT * FROM user,course,user_course WHERE user.id=user_course.user AND user_course.course=course.id
		[course,course.id,user.id,course.name,user.name,teacher_id,user]
		-------------
		(1,1,1,'Math','Jimmy',3,1)
		(1,1,2,'Math','Gladys',3,2)
		(2,2,1,'English','Jimmy',4,1)
		testDB> UPDATE SET name='JimmyJimmy' where name='Jimmy'
		*** Error: syntax error
		testDB> UPDATE user SET name='JimmyJimmy' where name='Jimmy'
		* Success: 1 records updated in table 'user'.
		testDB> show user
		Table 'user': (
		        id INT NOT NULL,
		        name VARCHAR(255),
		)
		#record = 5
		------------
		(1, 'JimmyJimmy')
		(2, 'Gladys')
		(3, 'xx')
		(4, 'yy')
		(5, NULL)
		testDB> SELECT * FROM user,course,user_course WHERE user.id=user_course.user AND user_course.course=course.id
		[course,course.id,user.id,course.name,user.name,teacher_id,user]
		-------------
		(1,1,1,'Math','JimmyJimmy',3,1)
		(1,1,2,'Math','Gladys',3,2)
		(2,2,1,'English','JimmyJimmy',4,1)
		testDB> exit
		Database manage system quits normally.

