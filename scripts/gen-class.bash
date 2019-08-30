#/usr/bin/bash

name=$1
#name_uc=$(echo $name | sed -r 's/([A-Z])/_\L\1/g' | sed 's/^_//')
name_uc=${name^^}

echo "#ifndef VIDREVOLT_$name_uc""_H_
#define VIDREVOLT_$name_uc""_H_

namespace vidrevolt {
    class $name {
        public:

    };
}

#endif
" >> common/$name.h

echo "#include \"$name.h\"

namespace vidrevolt {

}
" >> common/$name.cpp

