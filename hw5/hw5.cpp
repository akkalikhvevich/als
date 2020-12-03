// hw5.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//



#include <iostream>
#include <fstream>
#include <gdal.h>
#include<boost/geometry.hpp>
#include <ogrsf_frmts.h>

using namespace std;
using namespace boost::geometry::model;
using namespace boost::geometry::cs;
static const int maxFeatures = 1000;
int main(int argc, char* argv[])
{
    ifstream f1;
    f1.open(argv[2]);
    double xmin, xmax, ymin, ymax;
    f1 >> xmin >> ymin >> xmax >> ymax;
    box < point <double, 2, cartesian>>rectangle 
    (point<double, 2, cartesian>({ xmin, ymin }), point<double, 2, cartesian>({ xmax,ymax }));

    GDALAllRegister();
    GDALDataset* dataset = static_cast<GDALDataset*>(GDALOpenEx(
        R"(argv[1])",
        GDAL_OF_VECTOR,
        nullptr, nullptr, nullptr));

    if (dataset == nullptr) {
        cout << "Cannot open file!" << endl;
        return -1;
    }
   
    boost::geometry::index::rtree<pair<box < point <double, 2, cartesian>>, int>,
        boost::geometry::index::quadratic<8, 4>> rtree;
    const std::shared_ptr<OGREnvelope> env(new OGREnvelope);
    
    for (auto&& layer : dataset->GetLayers()) {
        for (auto&& feature : layer) {
            auto* geometry = feature->GetGeometryRef();
            
            geometry->getEnvelope(env.get());
            
            box < point <double, 2, cartesian>>next_box 
            (point<double, 2, cartesian>({ env->MinX,env->MinY }), point<double, 2, cartesian>({ env->MaxX,env->MaxY })) ;

            pair  <box < point <double, 2, cartesian>>, int> next_pair (next_box, feature->GetFieldAsInteger(0));
            rtree.insert(next_pair);
            
        }
    }
    vector < pair  <box < point <double, 2, cartesian>>, int>> result;
    rtree.query(boost::geometry::index::intersects(rectangle), back_inserter(result));
    ofstream out;
    out.open(argv[3]);
    for (auto p : result)
    {
        out << p.second << endl;
    }
    out << endl;
    out.close();
    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
