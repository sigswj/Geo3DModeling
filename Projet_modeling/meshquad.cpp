#include "meshquad.h"
#include "matrices.h"
#include <QDebug>

MeshQuad::MeshQuad():
    m_nb_ind_edges(0),m_nb_ind_quad(0)
{

}


void MeshQuad::gl_init()
{
	m_shader_flat = new ShaderProgramFlat();
	m_shader_color = new ShaderProgramColor();

	//VBO
	glGenBuffers(1, &m_vbo);

	//VAO
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glEnableVertexAttribArray(m_shader_flat->idOfVertexAttribute);
	glVertexAttribPointer(m_shader_flat->idOfVertexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &m_vao2);
	glBindVertexArray(m_vao2);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glEnableVertexAttribArray(m_shader_color->idOfVertexAttribute);
	glVertexAttribPointer(m_shader_color->idOfVertexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);


	//EBO indices
	glGenBuffers(1, &m_ebo);
	glGenBuffers(1, &m_ebo2);
}

void MeshQuad::gl_update()
{
	//VBO
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * m_points.size() * sizeof(GLfloat), &(m_points[0][0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	std::vector<int> tri_indices;
	convert_quads_to_tris(m_quad_indices,tri_indices);

	//EBO indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,tri_indices.size() * sizeof(int), &(tri_indices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	std::vector<int> edge_indices;
	convert_quads_to_edges(m_quad_indices,edge_indices);
	m_nb_ind_edges = edge_indices.size();

    m_nb_ind_quad = m_quad_indices.size()/4;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_nb_ind_edges * sizeof(int), &(edge_indices[0]), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



void MeshQuad::set_matrices(const Mat4& view, const Mat4& projection)
{
	viewMatrix = view;
	projectionMatrix = projection;
}

void MeshQuad::draw(const Vec3& color)
{

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);

	m_shader_flat->startUseProgram();
	m_shader_flat->sendViewMatrix(viewMatrix);
	m_shader_flat->sendProjectionMatrix(projectionMatrix);
	glUniform3fv(m_shader_flat->idOfColorUniform, 1, glm::value_ptr(color));
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ebo);
	glDrawElements(GL_TRIANGLES, 3*m_quad_indices.size()/2,GL_UNSIGNED_INT,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	m_shader_flat->stopUseProgram();

	glDisable(GL_POLYGON_OFFSET_FILL);

	m_shader_color->startUseProgram();
	m_shader_color->sendViewMatrix(viewMatrix);
	m_shader_color->sendProjectionMatrix(projectionMatrix);
	glUniform3f(m_shader_color->idOfColorUniform, 0.0f,0.0f,0.0f);
	glBindVertexArray(m_vao2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ebo2);
	glDrawElements(GL_LINES, m_nb_ind_edges,GL_UNSIGNED_INT,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	m_shader_color->stopUseProgram();
}

//Clear les listes
void MeshQuad::clear()
{
    m_points.clear();
    m_quad_indices.clear();
}

//Ajoute un point
int MeshQuad::add_vertex(const Vec3& P)
{
    m_points.push_back(P);
    return m_points.size()-1;
}

//Ajoute un quadrilatère
void MeshQuad::add_quad(int i1, int i2, int i3, int i4)
{
    m_quad_indices.push_back(i1);
    m_quad_indices.push_back(i2);
    m_quad_indices.push_back(i3);
    m_quad_indices.push_back(i4);
}


//Convert les quad en triangles
void MeshQuad::convert_quads_to_tris(const std::vector<int>& quads, std::vector<int>& tris)
{
    // a------b
    // |      |
    // |      |
    // d------c

    tris.clear();
    tris.reserve(3*quads.size()/2); // 1 quad = 4 indices -> 2 tris = 6 indices d'ou ce calcul (attention division entiere)


    for(int i=0 ; i<quads.size();i=i+4)
    {

        //Triangle ABD
        tris.push_back(quads[i]);
        tris.push_back(quads[i+1]);
        tris.push_back(quads[i+3]);

        //Triangle BCD
        tris.push_back(quads[i+1]);
        tris.push_back(quads[i+2]);
        tris.push_back(quads[i+3]);

    }

    // Pour chaque quad on genere 2 triangles
    // Attention a respecter l'orientation des triangles
}

//Convert les quad en edges
void MeshQuad::convert_quads_to_edges(const std::vector<int>& quads, std::vector<int>& edges)
{
    // a------b
    // |      |
    // |      |
    // d------c


    edges.clear();
    edges.reserve(quads.size()); // ( *2 /2 !)

    bool e1=true , e2=true , e3=true , e4=true;

    for(int i=0 ; i<quads.size();i = i+4)
    {

        for(int j=0 ; j<edges.size() ; j = j+2)
        {

            if(edges[j] == quads[i] && edges[j+1]== quads[i+1] || edges[j] == quads[i+1] && edges[j+1]== quads[i] ){
                e1 = false;
            }
            if(edges[j] == quads[i+1] && edges[j+1]== quads[i+2] || edges[j] == quads[i+2] && edges[j+1]== quads[i+1]){
                e2=false;
            }
            if(edges[j] == quads[i+2] && edges[j+1]== quads[i+3] || edges[j] == quads[i+3] && edges[j+1]== quads[i+2]){
                e3=false;
            }
            if(edges[j] == quads[i+3] && edges[j+1]== quads[i] || edges[j] == quads[i] && edges[j+1]== quads[i+3] ){
                e4=false;
            }

        }

        if(e1 == true ){
            //Arete AB
            edges.push_back(quads[i]);
            edges.push_back(quads[i+1]);
        }

        if(e2==true){
            //Arete BC
            edges.push_back(quads[i+1]);
            edges.push_back(quads[i+2]);
        }

        if(e3==true){
            //Arete CD
            edges.push_back(quads[i+2]);
            edges.push_back(quads[i+3]);
        }

        if(e4==true){
            //Arete DA
            edges.push_back(quads[i+3]);
            edges.push_back(quads[i]);
        }

         e1=true;
         e2=true;
         e3=true;
         e4=true;

    }


    // Pour chaque quad on genere 4 aretes, 1 arete = 2 indices.
    // Mais chaque arete est commune a 2 quads voisins !


    // Comment n'avoir qu'une seule fois chaque arete ?
    // Boucle in the boucle


}

//Créé un cube
void MeshQuad::create_cube()
{
    clear();

    //8 Sommets
    int a = add_vertex(Vec3(-1,-1,1));
    int b = add_vertex(Vec3(1,-1,1));
    int c = add_vertex(Vec3(1,-1,-1));
    int d = add_vertex(Vec3(-1,-1,-1));
    int e = add_vertex(Vec3(-1,1,-1));
    int f = add_vertex(Vec3(-1,1,1));
    int g = add_vertex(Vec3(1,1,1));
    int h = add_vertex(Vec3(1,1,-1));


    //6 faces
    add_quad(a,d,c,b);
    add_quad(e,f,g,h);
    add_quad(h,c,d,e);
    add_quad(e,d,a,f);
    add_quad(f,a,b,g);
    add_quad(g,b,c,h);

    gl_update();
}

//Calcul de la normal du quad
Vec3 MeshQuad::normal_of_quad(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D)
{
    //Vecteur directeur
    Vec3 AD , AB , CB , CD;

    //Triangle ADC
    //AD
    AD = D-A;
    //Triangle ABC
    //AB
    AB = B-A;

    //Triangle CBD
    //CB
    CB = B-C;

    //CD
    CD = D-C;


    //Produit vectoriel
    Vec3 produitABAD , produitCBCD;

    //AD ^ AC
    produitABAD = glm::cross(AB,AD);

    //CB  ^ CD
    produitCBCD = glm::cross(CB,CD);

    //Moyenne des deux
    Vec3 moyenne ;
    moyenne.x = produitABAD.x+produitCBCD.x/2;
    moyenne.y = produitABAD.y+produitCBCD.y/2;
    moyenne.z = produitABAD.z+produitCBCD.z/2;

    moyenne = glm::normalize(moyenne);
    // Attention a l'ordre des points !
    // le produit vectoriel n'est pas commutatif U ^ V = - V ^ U
    // ne pas oublier de normaliser le resultat.

    return  moyenne;
}

//Calcul de l'air d'un quad
float MeshQuad::area_of_quad(const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D)
{
    //Vecteur directeur
    Vec3 AD , AB , CB , CD;


    //Triangle ADC
    //AD
    AD = D-A;
    //Triangle ABC
    //AB
    AB = B-A;

    //Triangle CBD
    //CB
    CB = B-C;

    //CD
    CD = D-C;

    //Produit vectoriel
    Vec3 produitABAD , produitCBCD;

    //AB ^ AC
    produitABAD = glm::cross(AB,AD);

    //AD ^ AC
    produitCBCD = glm::cross(CB,CD);

    // aire du tri = 1/2 aire parallelogramme
    double airABD =  0.5*glm::length(produitABAD);
    double airCBD =  0.5*glm::length(produitCBCD);

    // aire du quad - aire tri + aire tri
    float airquad = airABD+airCBD;

	// aire parallelogramme: cf produit vectoriel

    return airquad;
}

//Regarde si un point est dans le quad
bool MeshQuad::is_points_in_quad(const Vec3& P, const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D)
{


      Vec3 normale = normal_of_quad(A,B,C,D);
      Vec3 AB = cross(normale,B-A);
      Vec3 BC = cross(normale,C-B);
      Vec3 CD = cross(normale,D-C);
      Vec3 DA = cross(normale,A-D);

      double ScalAB = dot(AB,P);
      double ScaleBC = dot(BC,P);
      double ScaleCD = dot(CD,P);
      double ScaleDA = dot(DA,P);

      double da = dot(AB,A);
      double db = dot(BC,B);
      double dc = dot(CD,C);
      double dd = dot(DA,D);

      if(ScalAB>da && ScaleBC > db && ScaleCD > dc && ScaleDA > dd)
          return true;
      return false;

}

//Regarde si un rayon intersect un quad
bool MeshQuad::intersect_ray_quad(const Vec3& P, const Vec3& Dir, int q, Vec3& inter)
{
	// recuperation des indices de points
    int i1 = m_quad_indices[q*4];
    int i2 = m_quad_indices[q*4+1];
    int i3 = m_quad_indices[q*4+2];
    int i4 = m_quad_indices[q*4+3];

	// recuperation des points
    //Point ABCD du quad
    Vec3 A = m_points[i1];
    Vec3 B = m_points[i2];
    Vec3 C = m_points[i3];
    Vec3 D = m_points[i4];

	// calcul de l'equation du plan (N+d)
    Vec3 normal = normal_of_quad(A,B,C,D);
    float dequation = glm::dot(normal,A);

	// I = P + alpha*Dir est dans le plan => calcul de alpha
    //trouver alpha :

    float alpha = (dequation - glm::dot(normal,P))/(glm::dot(normal,Dir));

	// alpha => calcul de I
    Vec3 I = P + alpha*Dir;
    // I dans le quad ?
    //regarder si I dans le plan
    if(glm::dot(normal,I) != dequation)
        return false;

    // si dans le plan alors test dans le quad
    if(is_points_in_quad(I,A,B,C,D))
    {
        inter = I;
         return true;
    }

    return false;
}

//Regarde la plus proche intersection
int MeshQuad::intersected_visible(const Vec3& P, const Vec3& Dir)
{
    int inter = -1;
    Vec3 intersection = Vec3(0,0,0);
    Vec3 precintersection = Vec3(0,0,0);

    // on parcours tous les quads
    for(int i =0 ; i< m_quad_indices.size() ; i = i+4)
    {
         // on teste si il y a intersection avec le rayon
        if(intersect_ray_quad(P,Dir,i/4,intersection))
        {

            if(i/4 == 0)
            {
                precintersection = intersection;
                inter = i/4;
            }
            else
            {
                if(glm::distance(P, intersection) < glm::distance(P, precintersection))
                {
                    precintersection = intersection;
                    inter = i/4;
                }
            }
        }
    }

    // on garde le plus proche (de P)

    return inter;

}


Mat4 MeshQuad::local_frame  (int q)
{
    // Repere locale = Matrice de transfo avec
    // les trois premieres colones: X,Y,Z locaux
    // la derniere colonne l'origine du repere


    // ici Z = N et X = AB
    // Origine le centre de la face
    // longueur des axes : [AB]/2

    // recuperation des indices de points
    int i1 = m_quad_indices[q*4];
    int i2 = m_quad_indices[q*4+1];
    int i3 = m_quad_indices[q*4+2];
    int i4 = m_quad_indices[q*4+3];

    // recuperation des points
    Vec3 A = m_points[i1];
    Vec3 B = m_points[i2];
    Vec3 C = m_points[i3];
    Vec3 D = m_points[i4];

    // calcul de Z:N puis de X:arete on en deduit Y
    Vec3 N = normal_of_quad(A,B,C,D);
    Vec3 X = normalize(B-A);
    Vec3 Y = cross(N,X);

    // calcul du centre
    Vec3 centre = A+B+C+D;
    centre.x = centre.x / 4;
    centre.y = centre.y / 4;
    centre.z = centre.z / 4;

    // calcul de la taille

    float taille = length(B-A)/2;

    // calcul de la matrice
    Mat4 res = Mat4(Vec4(X,0),Vec4(Y,0),Vec4(N,0),Vec4(centre,1));
    res*scale(taille,taille,taille);


    return res;
}


void MeshQuad::extrude_quad(int q)
{
    // recuperation des indices de points
    int i1 = m_quad_indices[q*4];
    int i2 = m_quad_indices[q*4+1];
    int i3 = m_quad_indices[q*4+2];
    int i4 = m_quad_indices[q*4+3];

    // recuperation des points
    //Point ABCD du quad
    Vec3 A = m_points[i1];
    Vec3 B = m_points[i2];
    Vec3 C = m_points[i3];
    Vec3 D = m_points[i4];

    // calcul de la normale
    Vec3 normal = normal_of_quad(A,B,C,D);

    // calcul de la hauteur
    float hauteur = sqrt(area_of_quad(A,B,C,D));
    Vec3 A2,B2,C2,D2;

    A2 = A+(hauteur*normal);
    B2 = B+(hauteur*normal);
    C2 = C+(hauteur*normal);
    D2 = D+(hauteur*normal);




    int a2 = add_vertex(A2);
    int b2 = add_vertex(B2);
    int c2 = add_vertex(C2);
    int d2 = add_vertex(D2);

    // on ajoute les 4 quads des cotes
    add_quad(d2,c2,i3,i4);
    add_quad(c2,b2,i2,i3);
    add_quad(b2,a2,i1,i2);
    add_quad(a2,d2,i4,i1);



    // on remplace le quad initial par le quad du dessu
    m_quad_indices[q*4] = a2;
    m_quad_indices[q*4+1] = b2;
    m_quad_indices[q*4+2] = c2;
    m_quad_indices[q*4+3] = d2;

    gl_update();


}

void MeshQuad::decale_quad(int q, float d)
{
    // recuperation des indices de points
    int i1 = m_quad_indices[q*4];
    int i2 = m_quad_indices[q*4+1];
    int i3 = m_quad_indices[q*4+2];
    int i4 = m_quad_indices[q*4+3];

    // recuperation des (references de) points
    Vec3 *A = &m_points[i1];
    Vec3 *B = &m_points[i2];
    Vec3 *C = &m_points[i3];
    Vec3 *D = &m_points[i4];

    // calcul de la normale
    Vec3 normal = normal_of_quad(*A,*B,*C,*D);

    // modification des points
    float decal = sqrt(area_of_quad(*A,*B,*C,*D)) * d;

    *A = *A+(decal*normal);
    *B = *B+(decal*normal);
    *C = *C+(decal*normal);
    *D = *D+(decal*normal);


    gl_update();
}

void MeshQuad::shrink_quad(int q, float s)
{
    // recuperation des indices de points
    int i1 = m_quad_indices[q*4];
    int i2 = m_quad_indices[q*4+1];
    int i3 = m_quad_indices[q*4+2];
    int i4 = m_quad_indices[q*4+3];

    // recuperation des (references de) points
    Vec3 *A = &m_points[i1];
    Vec3 *B = &m_points[i2];
    Vec3 *C = &m_points[i3];
    Vec3 *D = &m_points[i4];

	// calcul du centre
    Vec3 centre = *A+*B+*C+*D;
    centre.x = centre.x / 4;
    centre.y = centre.y / 4;
    centre.z = centre.z / 4;

	 // modification des points
    *A += (*A-centre) * s;
    *B += (*B-centre) * s;
    *C += (*C-centre) * s;
    *D += (*D-centre) * s;

	gl_update();
}

void MeshQuad::tourne_quad(int q, float a)
{
    // recuperation des indices de points
    int i1 = m_quad_indices[q*4];
    int i2 = m_quad_indices[q*4+1];
    int i3 = m_quad_indices[q*4+2];
    int i4 = m_quad_indices[q*4+3];

    // recuperation des (references de) points
    Vec3 *A = &m_points[i1];
    Vec3 *B = &m_points[i2];
    Vec3 *C = &m_points[i3];
    Vec3 *D = &m_points[i4];

	// generation de la matrice de transfo:
    Vec4 AR = Vec4(*A,1);
    Vec4 BR = Vec4(*B,1);
    Vec4 CR = Vec4(*C,1);
    Vec4 DR = Vec4(*D,1);

    // tourne autour du Z de la local frame
    Mat4 transfo ;
    transfo = local_frame(q)*rotateZ(a)*glm::inverse(local_frame(q));

    //transformation avec les coordonées homogènes
    AR = transfo*AR;
    BR = transfo*BR;
    CR = transfo*CR;
    DR = transfo*DR;

    //Modification des coordonées existante
    A->x = AR.x;
    A->y = AR.y;
    A->z = AR.z;

    B->x = BR.x;
    B->y = BR.y;
    B->z = BR.z;

    C->x = CR.x;
    C->y = CR.y;
    C->z = CR.z;

    D->x = DR.x;
    D->y = DR.y;
    D->z = DR.z;

	gl_update();
}

