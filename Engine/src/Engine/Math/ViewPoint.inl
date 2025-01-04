namespace Math
{

void FViewPoint::ComputeProjectionMatrix()
{
    const float tanHalfFOV = std::tan(m_FOV / 2);

    FMatrix4 NewProjectionMatrix;
    m_ProjectionMatrix[0][0] = 1 / (m_AspectRatio * tanHalfFOV);
    m_ProjectionMatrix[1][1] = 1 / tanHalfFOV;
    m_ProjectionMatrix[2][2] = (m_Far + m_Near) / (m_Near - m_Far);
    m_ProjectionMatrix[2][3] = -1;
    m_ProjectionMatrix[3][2] = (2 * m_Far * m_Near) / (m_Near - m_Far);

    m_ProjectionMatrix = NewProjectionMatrix;
}

}    // namespace Math
