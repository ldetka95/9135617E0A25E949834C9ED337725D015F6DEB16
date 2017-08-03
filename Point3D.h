#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include <math.h>
#include <list>

using namespace std;

struct ___Point3D {
    public:
    double x, y, z;
};

typedef struct ___Point3D Point3D;

Point3D Point( double x, double y, double z ) {
    Point3D ret;
    ret.x = x;
    ret.y = y;
    ret.z = z;
    return ret;
}

Point3D grndVector = { 0.0, -0.05, 0.0 };

Point3D SubtractPoint( Point3D a, Point3D b ) {
    Point3D c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
    return c;
}

Point3D AddPoint( Point3D a, Point3D b ) {
    Point3D c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
    return c;
}

void ScalePoint3D( Point3D* p, double scale ) {
    if ( p ) {
        p -> x *= scale;
        p -> y *= scale;
        p -> z *= scale;
    }
}

void SignalScale( Point3D* p, double scale ) {
    if ( p ) {
        if ( p -> x >= 0.0 ) {
            p -> x = scale;
        } else {
            p -> x = - scale;
        }
        if ( p -> y >= 0.0 ) {
            p -> y = scale;
        } else {
            p -> y = - scale;
        }
        if ( p -> z >= 0.0 ) {
            p -> z = scale;
        } else {
            p -> z = - scale;
        }
    }
}

Point3D RoundUp( Point3D p ) {
    Point3D r;
    r.x = round( p.x );
    r.y = round( p.y );
    r.z = round( p.z );
    return r;
}

Point3D Floor( Point3D p ) {
    Point3D r;
    r.x = floor( p.x );
    r.y = floor( p.y );
    r.z = floor( p.z );
    return r;
}

double Dist3D( Point3D a, Point3D b ) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double dz = b.z - a.z;
    return sqrt( dx * dx + dy * dy + dz * dz );
}

Point3D Balance( Point3D a, Point3D b, double c ) {
    Point3D r;
    r.x = b.x * c + a.x * ( 1.0 - c );
    r.y = b.y * c + a.y * ( 1.0 - c );
    r.z = b.z * c + a.z * ( 1.0 - c );
    return r;
}

Point3D Min( Point3D a, Point3D b ) {
    Point3D pzero = Point( 0.0, 0.0, 0.0 );
    if ( Dist3D( pzero, a ) < Dist3D( pzero, b ) ) {
        return a;
    }
    return b;
}

Point3D Max( Point3D a, Point3D b ) {
    Point3D pzero = Point( 0.0, 0.0, 0.0 );
    if ( Dist3D( pzero, a ) < Dist3D( pzero, b ) ) {
        return b;
    }
    return a;
}

Point3D Multiply3D( Point3D a, Point3D b ) {
    Point3D c;
    c.x = a.x * b.x;
    c.y = a.y * b.y;
    c.z = a.z * b.z;
    return c;
}

double absf( double x ) {
    if ( x < 0 ) {
        return -x;
    }
    return x;
}

double pHeight = 1.7;
double pArmHeight = 1.5;

class Timer {
    public:
    Timer( int Interval ) {
        interval = Interval;
        lastTime = glutGet( GLUT_ELAPSED_TIME );
        lastTimePaused = lastTime;
        generateNewInterval = false;
        paused = false;
        registerTimer();
    }
    Timer( int IntervalMin, int IntervalMax ) {
        iMin = IntervalMin;
        iMax = IntervalMax;
        interval = rand() % ( iMax - iMin + 1 ) + iMin;
        lastTime = glutGet( GLUT_ELAPSED_TIME );
        lastTimePaused = lastTime;
        generateNewInterval = true;
        paused = false;
        registerTimer();
    }
    ~Timer() {
        removeTimer();
    }
    bool Tick() {
        if ( paused ) {
            return false;
        }
        unsigned int current = glutGet( GLUT_ELAPSED_TIME );
        if ( ( lastTime + interval ) <= current ) {
            lastTime = current;
            if ( generateNewInterval ) {
                interval = rand() % ( iMax - iMin + 1 ) + iMin;
            }
            return true;
        }
        return false;
    }
    int Remaining() {
        int current = glutGet( GLUT_ELAPSED_TIME );
        return ( lastTime + interval ) - current;
    }
    void ChangeInterval( int newInterval ) {
        interval = newInterval;
    }
    void Pause() {
        paused = true;
        lastTimePaused = glutGet( GLUT_ELAPSED_TIME );
    }
    void Resume() {
        paused = false;
        lastTime += ( glutGet( GLUT_ELAPSED_TIME ) - lastTimePaused );
    }
    bool IsPaused() {
        return paused;
    }
    double GetInterval() {
        return interval;
    }
    void ForceTick() {
        lastTime -= Remaining();
    }
    void RestartTick() {
        lastTime = glutGet( GLUT_ELAPSED_TIME );
    }
    static double Current() {
        return ( double )( glutGet( GLUT_ELAPSED_TIME ) / 1000.0 );
    }
    private:
    void registerTimer() {
        allTimers.push_back( this );
        //printf( " > Registering timer %d: %p\n", allTimers.size(), this );
    }
    void removeTimer() {
        int timerNum = allTimers.size();
        for ( int i = 0; i < timerNum; i++ ) {
            Timer* t = allTimers.back();
            allTimers.pop_back();
            if ( t == this ) {
                break;
            }
            allTimers.push_front( t );
        }
    }
    unsigned int interval, lastTime, lastTimePaused;
    unsigned int iMin, iMax;
    bool generateNewInterval, paused;
    static list< Timer* > allTimers;
};

list< Timer* > Timer::allTimers;

#define GEN___DENSITY 10000

double Random( double range ) {
    return double( rand() % GEN___DENSITY ) * range / double( GEN___DENSITY );
}

double Random( double a, double b ) {
    double range = b - a;
    return ( double( rand() % GEN___DENSITY ) * range / double( GEN___DENSITY ) ) + a;
}

int Random( int Rmin, int Rmax ) {
    return rand() % ( Rmax - Rmin + 1 ) + Rmin;
}

double Slap( double x, double a, double b ) {
    if ( x > b ) {
        return b;
    }
    if ( x < a ) {
        return a;
    }
    return x;
}

template < class T >
    class Stack {
        public:
        Stack() {
            head = NULL;
        }
        ~Stack() {
            Clear();
        }
        void Clear() {
            if ( head ) {
                delete head;
            }
            head = NULL;
        }
        void Push( T data ) {
            StackElement* element = new StackElement( data );
            element -> next = head;
            head = element;
        }
        T* Pop() {
            T* ret = NULL;
            if ( head ) {
                if ( head -> data ) {
                    ret = head -> data;
                    StackElement* grinder = head;
                    head = head -> next;
                    grinder -> next = NULL;
                    delete grinder;
                }
            }
            return ret;
        }
        void Head() {
            current = head;
            currentPrev = NULL;
        }
        bool Next() {
            if ( current ) {
                currentPrev = current;
                current = current -> next;
                return true;
            }
            return false;
        }
        T* CurrentGet() {
            if ( current ) {
                return ( current -> data );
            }
            return NULL;
        }
        void CurrentRem() {
            if ( current ) {
                StackElement* grinder = current;
                current = current -> next;
                if ( currentPrev ) {
                    currentPrev -> next = current;
                } else {
                    head = current;
                }
                grinder -> next = NULL;
                delete grinder;
            }
        }
        private:
        class StackElement {
            public:
            StackElement( T dataCpy ) {
                data = ( T* )( malloc( sizeof( T ) ) );
                ( *data ) = dataCpy;
                next = NULL;
            }
            ~StackElement() {
                //free( data );
                if ( next ) {
                    delete next;
                }
            }
            T* data;
            StackElement* next;
        };
        StackElement* head;
        StackElement* current;
        StackElement* currentPrev;
    };


