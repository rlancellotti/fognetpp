//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

namespace fog;


//CREATO PER POTER AGGIUNGER LA INFO SULLA QUERYID NELLA RISPOSTA AL PROBE
//SE VOGLIAMO MANTENERE SOLO IL LOADUPDATE POSSIAMO AGGIUNGERE LI LA QUERYID
//E SETTARLO A -1 SE LA INFO NON CI SERVE
packet ProbeAnswer {
    double load;
    int queryId;
    int appId;
}
